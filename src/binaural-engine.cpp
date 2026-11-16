#include "binaural-engine.h"

#include <algorithm>
#include <cmath>

#include "api/resonance_audio_api.h"
#include "dsp/resampler.h"
#include "platforms/common/room_effects_utils.h"
#include "platforms/common/room_properties.h"

namespace sbn {

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr std::array<vraudio::RenderingMode, 3> kModes{
	vraudio::kBinauralLowQuality, vraudio::kBinauralMediumQuality, vraudio::kBinauralHighQuality};

// A medium room with plaster walls, a wooden floor and an absorbing ceiling. The Room parameter sets how
// much of this room the source excites.
vraudio::RoomProperties roomPreset()
{
	vraudio::RoomProperties room;
	room.dimensions[0] = 6.0f;
	room.dimensions[1] = 3.0f;
	room.dimensions[2] = 8.0f;
	const vraudio::MaterialName walls = vraudio::kPlasterSmooth;
	room.material_names[0] = walls;
	room.material_names[1] = walls;
	room.material_names[2] = vraudio::kParquetOnConcrete;
	room.material_names[3] = vraudio::kAcousticCeilingTiles;
	room.material_names[4] = walls;
	room.material_names[5] = walls;
	return room;
}

} // namespace

BinauralEngine::BinauralEngine() = default;
BinauralEngine::~BinauralEngine() = default;

int BinauralEngine::clampQuality(int quality)
{
	return std::clamp(quality, 0, int(kQualityNames.size()) - 1);
}

bool BinauralEngine::isSupportedSampleRate(double sampleRate)
{
	const double rounded = std::round(sampleRate);
	if (rounded != sampleRate || rounded < 8000.0 || rounded > 384000.0)
		return false;
	return vraudio::Resampler::AreSampleRatesSupported(48000, int(rounded));
}

bool BinauralEngine::prepare(double sampleRate)
{
	release();
	ready_ = true;
	reset();
	if (!isSupportedSampleRate(sampleRate))
		return false;
	api_.reset(vraudio::CreateResonanceAudioApi(2, kBlock, int(sampleRate)));
	if (!api_)
		return false;
	for (size_t q = 0; q < kModes.size(); ++q) {
		const int id = api_->CreateSoundObjectSource(kModes[q]);
		sources_[q] = id;
		api_->SetSourceDistanceModel(id, vraudio::kLogarithmic, float(kMinDistance), float(kMaxDistance));
		api_->SetSoundObjectNearFieldEffectGain(id, 1.0f);
	}
	const vraudio::RoomProperties room = roomPreset();
	api_->SetReflectionProperties(vraudio::ComputeReflectionProperties(room));
	api_->SetReverbProperties(vraudio::ComputeReverbProperties(room));
	api_->EnableRoomEffects(true);
	return true;
}

void BinauralEngine::release()
{
	api_.reset();
	ready_ = false;
	sources_ = {-1, -1, -1};
}

void BinauralEngine::reset()
{
	input_.fill(0.0f);
	rendered_.fill(0.0f);
	output_.fill(0.0f);
	fill_ = 0;
	paramsChanged_ = true;
}

void BinauralEngine::setParams(double azimuth, double elevation, double distance, double room, int quality)
{
	azimuth = std::clamp(azimuth, -180.0, 180.0);
	elevation = std::clamp(elevation, -90.0, 90.0);
	distance = std::clamp(distance, kMinDistance, kMaxDistance);
	room = std::clamp(room, 0.0, 1.0);
	quality = clampQuality(quality);
	if (azimuth != azimuth_ || elevation != elevation_ || distance != distance_ || room != room_ ||
	    quality != quality_) {
		azimuth_ = azimuth;
		elevation_ = elevation;
		distance_ = distance;
		room_ = room;
		quality_ = quality;
		paramsChanged_ = true;
	}
}

void BinauralEngine::applyParams()
{
	// Resonance Audio: x to the right, y up, -z to the front of the listener at the origin.
	const double az = azimuth_ * kPi / 180.0;
	const double el = elevation_ * kPi / 180.0;
	const auto x = float(distance_ * std::cos(el) * std::sin(az));
	const auto y = float(distance_ * std::sin(el));
	const auto z = float(-distance_ * std::cos(el) * std::cos(az));
	for (const int id : sources_) {
		api_->SetSourcePosition(id, x, y, z);
		api_->SetSourceRoomEffectsGain(id, float(room_));
	}
	paramsChanged_ = false;
}

void BinauralEngine::renderBlock()
{
	if (!api_) {
		for (size_t i = 0; i < kBlock; ++i)
			rendered_[2 * i] = rendered_[2 * i + 1] = input_[i];
		return;
	}
	if (paramsChanged_)
		applyParams();
	// Only the source of the selected quality gets audio. The others get silence so that a quality change
	// starts them from silence instead of from old audio.
	for (size_t q = 0; q < sources_.size(); ++q)
		api_->SetInterleavedBuffer(sources_[q], int(q) == quality_ ? input_.data() : silence_.data(), 1, kBlock);
	if (!api_->FillInterleavedOutputBuffer(2, kBlock, rendered_.data()))
		rendered_.fill(0.0f);
}

void BinauralEngine::process(const float *inL, const float *inR, float *outL, float *outR, uint32_t frames)
{
	if (!ready_) {
		std::fill(outL, outL + frames, 0.0f);
		std::fill(outR, outR + frames, 0.0f);
		return;
	}
	for (uint32_t i = 0; i < frames; ++i) {
		// Read the input before writing, because the host may process in place.
		const float mono = 0.5f * (inL[i] + inR[i]);
		outL[i] = output_[2 * fill_];
		outR[i] = output_[2 * fill_ + 1];
		input_[fill_] = mono;
		if (++fill_ == kBlock) {
			renderBlock();
			output_ = rendered_;
			fill_ = 0;
		}
	}
}

} // namespace sbn
