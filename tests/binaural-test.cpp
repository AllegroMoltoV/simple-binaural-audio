// Tests of the binaural engine: direction cues, the reported latency, independence from the host's block
// size, refusal of unsupported sample rates, and the step size when the direction jumps.
#include "binaural-engine.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <vector>

namespace {

int failures = 0;

void check(bool ok, const char *what, double sampleRate, double value = 0.0)
{
	std::printf("%s %s at %.0f Hz (value %.6g)\n", ok ? "ok  " : "FAIL", what, sampleRate, value);
	if (!ok)
		++failures;
}

constexpr double kRates[] = {44100.0, 48000.0, 96000.0};
constexpr double kPi = 3.14159265358979323846;

std::vector<float> noise(size_t frames, uint32_t seed)
{
	std::vector<float> x(frames);
	for (auto &v : x) {
		seed ^= seed << 13;
		seed ^= seed >> 17;
		seed ^= seed << 5;
		v = (float(seed >> 8) / float(1 << 24) * 2.0f - 1.0f) * 0.5f;
	}
	return x;
}

struct Stereo {
	std::vector<float> l, r;
};

Stereo run(double rate, const std::vector<float> &in, double azimuth, double elevation, uint32_t block,
	   double room = 0.0)
{
	sbn::BinauralEngine engine;
	engine.prepare(rate);
	engine.setParams(azimuth, elevation, 1.0, room, 2);
	Stereo out{std::vector<float>(in.size()), std::vector<float>(in.size())};
	for (size_t at = 0; at < in.size(); at += block) {
		const auto n = uint32_t(std::min<size_t>(block, in.size() - at));
		engine.process(in.data() + at, in.data() + at, out.l.data() + at, out.r.data() + at, n);
	}
	return out;
}

double rms(const std::vector<float> &x, size_t from)
{
	double sum = 0;
	for (size_t i = from; i < x.size(); ++i)
		sum += double(x[i]) * x[i];
	return std::sqrt(sum / double(x.size() - from));
}

// Lag of the right channel against the left, on low-passed copies, because the interaural time difference
// is a low-frequency cue. Negative: the right ear hears first.
int lowBandLag(Stereo s, double rate, size_t from)
{
	const float a = float(std::exp(-2.0 * kPi * 800.0 / rate));
	for (auto *ch : {&s.l, &s.r}) {
		float y1 = 0, y2 = 0;
		for (auto &v : *ch) {
			y1 = (1 - a) * v + a * y1;
			y2 = (1 - a) * y1 + a * y2;
			v = y2;
		}
	}
	const int maxLag = int(rate / 1000.0);
	int bestLag = 0;
	double best = -1e300;
	for (int lag = -maxLag; lag <= maxLag; ++lag) {
		double sum = 0;
		for (size_t i = from + size_t(maxLag); i + size_t(maxLag) < s.l.size(); ++i)
			sum += double(s.l[i]) * s.r[size_t(int(i) + lag)];
		if (sum > best) {
			best = sum;
			bestLag = lag;
		}
	}
	return bestLag;
}

void testDirection()
{
	for (double rate : kRates) {
		const auto in = noise(size_t(rate * 2.0), 3);
		const size_t from = size_t(rate / 4.0);
		const auto right = run(rate, in, 90.0, 0.0, 512);
		const auto left = run(rate, in, -90.0, 0.0, 512);
		const auto front = run(rate, in, 0.0, 0.0, 512);
		const double rightDb = 20.0 * std::log10(rms(right.r, from) / rms(right.l, from));
		const double leftDb = 20.0 * std::log10(rms(left.r, from) / rms(left.l, from));
		const double frontDb = 20.0 * std::log10(rms(front.r, from) / rms(front.l, from));
		check(rightDb > 6.0, "a source at +90 deg is louder in the right ear", rate, rightDb);
		check(leftDb < -6.0, "a source at -90 deg is louder in the left ear", rate, leftDb);
		check(std::fabs(frontDb) < 0.5, "a source in front is equally loud in both ears", rate, frontDb);
		// Human interaural time differences reach about 0.6 to 0.8 ms at 90 degrees.
		const double rightMs = 1000.0 * lowBandLag(right, rate, from) / rate;
		const double leftMs = 1000.0 * lowBandLag(left, rate, from) / rate;
		check(rightMs < -0.4, "a source at +90 deg reaches the right ear first", rate, rightMs);
		check(leftMs > 0.4, "a source at -90 deg reaches the left ear first", rate, leftMs);
	}
}

// The output starts exactly kBlock frames late, which is the latency the plug-in reports.
void testLatency()
{
	for (double rate : kRates) {
		std::vector<float> in(sbn::BinauralEngine::kBlock * 8, 0.0f);
		in[0] = 1.0f;
		const auto out = run(rate, in, 0.0, 0.0, 64);
		bool silentBefore = true;
		for (size_t i = 0; i < sbn::BinauralEngine::kBlock; ++i)
			silentBefore = silentBefore && out.l[i] == 0.0f && out.r[i] == 0.0f;
		float after = 0.0f;
		for (size_t i = sbn::BinauralEngine::kBlock; i < out.l.size(); ++i)
			after = std::max(after, std::fabs(out.l[i]));
		check(silentBefore, "no output before the reported latency", rate);
		check(after > 0.01f, "the impulse comes out after the reported latency", rate, after);
	}
}

double maxDifference(const Stereo &a, const Stereo &b)
{
	double worst = 0.0;
	for (size_t i = 0; i < a.l.size(); ++i)
		worst = std::max({worst, double(std::fabs(a.l[i] - b.l[i])), double(std::fabs(a.r[i] - b.r[i]))});
	return worst;
}

// The host's block size does not change the output. With Room above 0, Resonance Audio's reverb differs
// between two runs of the same input in the last bit (up to about 6e-8), so the comparison allows 1e-6.
void testBlockSizes()
{
	for (double rate : kRates) {
		const auto in = noise(size_t(rate), 5);
		for (double room : {0.0, 0.5}) {
			const auto reference = run(rate, in, 30.0, 20.0, 256, room);
			for (uint32_t block : {1u, 37u, 255u, 257u, 4096u}) {
				const double difference = maxDifference(run(rate, in, 30.0, 20.0, block, room), reference);
				check(difference <= 1e-6, "same output for every block size", rate, difference);
			}
		}
	}
}

void testSampleRates()
{
	for (double rate : {44100.0, 48000.0, 88200.0, 96000.0, 176400.0, 192000.0}) {
		sbn::BinauralEngine engine;
		check(engine.prepare(rate) && engine.spatialized(), "supported sample rate", rate);
	}
	// Resonance Audio aborts for these. The engine must not create the renderer, and passes the input
	// through as mono, delayed by the reported latency. 1234.5678 Hz is one of the rates of the VST 3 validator.
	const size_t block = sbn::BinauralEngine::kBlock;
	for (double rate : {47999.0, 44100.5, 1234.5678}) {
		sbn::BinauralEngine engine;
		const bool spatial = engine.prepare(rate);
		std::vector<float> l(block * 3, 0.25f), r(l.size(), 0.75f), outL(l.size()), outR(l.size());
		engine.process(l.data(), r.data(), outL.data(), outR.data(), uint32_t(l.size()));
		const bool delayed = outL[block - 1] == 0.0f && outL[block] == 0.5f && outR[block] == 0.5f;
		check(!spatial && !engine.spatialized() && delayed, "unsupported sample rate passes the input through", rate);
	}
	// Every rate the VST 3 validator tries (public.sdk/source/vst/testsuite/processing/processformat.cpp)
	// runs without aborting the process.
	for (double rate : {22050.0, 32000.0, 44100.0, 48000.0, 88200.0, 96000.0, 192000.0, 384000.0, 1234.5678,
			    12345.678, 123456.78, 1234567.8}) {
		sbn::BinauralEngine engine;
		engine.prepare(rate);
		engine.setParams(45.0, 10.0, 2.0, 0.5, 2);
		std::vector<float> in(block * 4, 0.1f), outL(in.size()), outR(in.size());
		engine.process(in.data(), in.data(), outL.data(), outR.data(), uint32_t(in.size()));
		check(std::isfinite(outL.back()) && std::isfinite(outR.back()), "runs at a rate of the VST 3 validator", rate,
		      engine.spatialized() ? 1.0 : 0.0);
	}
}

// A jump of the direction from the far left to the far right does not click: Resonance Audio crossfades
// between the old and the new direction within a block.
void testDirectionJump()
{
	for (double rate : kRates) {
		const size_t frames = size_t(rate);
		std::vector<float> in(frames);
		const double w = 2.0 * kPi * 440.0 / rate;
		for (size_t i = 0; i < frames; ++i)
			in[i] = float(0.5 * std::sin(w * double(i)));
		sbn::BinauralEngine engine;
		engine.prepare(rate);
		std::vector<float> l(frames), r(frames);
		for (size_t at = 0; at < frames; at += 128) {
			const double azimuth = (at / size_t(rate / 8)) % 2 == 0 ? -90.0 : 90.0;
			engine.setParams(azimuth, 0.0, 1.0, 0.0, 2);
			const auto n = uint32_t(std::min<size_t>(128, frames - at));
			engine.process(in.data() + at, in.data() + at, l.data() + at, r.data() + at, n);
		}
		double worstStep = 0.0, peak = 0.0;
		for (size_t i = frames / 8; i < frames; ++i) {
			worstStep = std::max({worstStep, double(std::fabs(l[i] - l[i - 1])), double(std::fabs(r[i] - r[i - 1]))});
			peak = std::max({peak, double(std::fabs(l[i])), double(std::fabs(r[i]))});
		}
		// A sine of peak A steps by at most A * w per sample. A jump without a crossfade steps by up to 2 A.
		const double bound = 2.0 * peak * w;
		check(worstStep <= bound, "no click when the direction jumps", rate, worstStep / (peak * w));
	}
}

} // namespace

int main()
{
	testDirection();
	testLatency();
	testBlockSizes();
	testSampleRates();
	testDirectionJump();
	std::printf(failures == 0 ? "all binaural engine tests passed\n" : "%d failures\n", failures);
	return failures == 0 ? 0 : 1;
}
