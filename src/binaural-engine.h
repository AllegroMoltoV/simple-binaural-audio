#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace vraudio {
class ResonanceAudioApi;
}

namespace sbn {

// Places one sound source around the listener with Resonance Audio and renders it for headphones.
// Resonance Audio processes fixed blocks of kBlock frames, so the engine buffers the host's blocks and
// delays the output by exactly kBlock frames.
class BinauralEngine {
public:
	static constexpr size_t kBlock = 256;
	static constexpr std::array<const char *, 3> kQualityNames{"Low", "Medium", "High"};
	static constexpr int kDefaultQuality = 2;
	static constexpr double kMinDistance = 0.25;
	static constexpr double kMaxDistance = 10.0;

	BinauralEngine();
	~BinauralEngine();
	BinauralEngine(const BinauralEngine &) = delete;
	BinauralEngine &operator=(const BinauralEngine &) = delete;

	// Resonance Audio aborts the process for sample rates its resampler cannot convert from 48 kHz.
	static bool isSupportedSampleRate(double sampleRate);

	// Allocates the renderer. Not real-time safe. At a sample rate Resonance Audio cannot handle, the engine
	// does not create the renderer and passes the input through as mono, delayed by the same kBlock frames,
	// because a host may activate the plug-in at any rate (the VST 3 validator tries 1234.5678 Hz).
	// Returns whether the input is spatialized.
	bool prepare(double sampleRate);
	void release();
	void reset();
	bool spatialized() const
	{
		return api_ != nullptr;
	}

	// Degrees, metres, 0 to 1, and 0 to 2 (Low, Medium, High).
	void setParams(double azimuth, double elevation, double distance, double room, int quality);

	// Reads stereo input and writes binaural stereo output. The buffers may be the same. Real-time safe.
	// Before prepare(), the output is silence.
	void process(const float *inL, const float *inR, float *outL, float *outR, uint32_t frames);

	static int clampQuality(int quality);

private:
	void renderBlock();
	void applyParams();

	std::unique_ptr<vraudio::ResonanceAudioApi> api_;
	bool ready_ = false;
	std::array<int, 3> sources_{-1, -1, -1};

	double azimuth_ = 0.0, elevation_ = 0.0, distance_ = 1.0, room_ = 0.0;
	int quality_ = kDefaultQuality;
	bool paramsChanged_ = true;

	std::array<float, kBlock> input_{};
	std::array<float, 2 * kBlock> rendered_{};
	std::array<float, 2 * kBlock> output_{};
	std::array<float, kBlock> silence_{};
	size_t fill_ = 0; // Frames in input_, which is also the read position in output_.
};

} // namespace sbn
