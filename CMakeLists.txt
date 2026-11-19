cmake_minimum_required(VERSION 3.21)

project(simple-binaural-audio VERSION 1.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Link the C runtime statically so users need no Visual C++ redistributable.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
# The static libraries below are linked into the plugin modules. Only the symbols marked for export,
# such as clap_entry, GetPluginFactory, and the VST 3 module entry points, stay visible.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

option(SBA_SANITIZE "Build the unit test with AddressSanitizer and UndefinedBehaviorSanitizer (GCC and Clang only)" OFF)

# Hardening applies to every compiled source, including the libraries that FetchContent adds below.
if(MSVC)
  add_compile_options(/GS /guard:cf)
else()
  add_compile_options(-fstack-protector-strong -D_FORTIFY_SOURCE=2)
endif()

include(FetchContent)
# Resonance Audio with the patches Qt applies, and the Eigen and pffft versions Qt uses with it, from the
# 3rdparty folder of qtmultimedia at a fixed commit. Upstream Resonance Audio is archived and its own build
# forces C++11 and takes Eigen and pffft from their master branches. SOURCE_SUBDIR points to a folder without
# a CMakeLists.txt, so only the sources are downloaded and Qt itself is not configured.
FetchContent_Declare(
  qtmultimedia
  URL https://github.com/qt/qtmultimedia/archive/5d5bb219c326e8ea796643e405ab6ae0a05483e6.tar.gz
  URL_HASH SHA256=36d21191242f650ef829c827f05736765dad4f3401b67bd33f6e489620fde0a8
  SOURCE_SUBDIR src/3rdparty
)
# clap-wrapper v0.17.0. It downloads the CLAP and VST 3 SDKs at the versions it pins.
set(CLAP_WRAPPER_DOWNLOAD_DEPENDENCIES TRUE CACHE BOOL "" FORCE)
set(CLAP_WRAPPER_BUILD_AUV2 FALSE CACHE BOOL "" FORCE)
FetchContent_Declare(
  clap-wrapper
  GIT_REPOSITORY https://github.com/free-audio/clap-wrapper.git
  GIT_TAG 806bba0a5f792e3966b99c1561ab9952d3316d7c
)
FetchContent_MakeAvailable(qtmultimedia clap-wrapper)

set(PRODUCT_NAME "Simple Binaural")

set(QT3P "${qtmultimedia_SOURCE_DIR}/src/3rdparty")
set(RA_TOP "${QT3P}/resonance-audio")
# The source list of Qt's own build, src/resonance-audio/CMakeLists.txt in qtmultimedia, without Qt's wrapper.
set(RA_SOURCES
  ${RA_TOP}/platforms/common/utils.cc
  ${RA_TOP}/resonance_audio/ambisonics/ambisonic_binaural_decoder.cc
  ${RA_TOP}/resonance_audio/ambisonics/ambisonic_lookup_table.cc
  ${RA_TOP}/resonance_audio/ambisonics/associated_legendre_polynomials_generator.cc
  ${RA_TOP}/resonance_audio/ambisonics/foa_rotator.cc
  ${RA_TOP}/resonance_audio/ambisonics/hoa_rotator.cc
  ${RA_TOP}/resonance_audio/ambisonics/stereo_from_soundfield_converter.cc
  ${RA_TOP}/resonance_audio/api/binaural_surround_renderer.cc
  ${RA_TOP}/resonance_audio/api/resonance_audio_api.cc
  ${RA_TOP}/resonance_audio/base/audio_buffer.cc
  ${RA_TOP}/resonance_audio/base/channel_view.cc
  ${RA_TOP}/resonance_audio/base/misc_math.cc
  ${RA_TOP}/resonance_audio/base/simd_utils.cc
  ${RA_TOP}/resonance_audio/base/spherical_angle.cc
  ${RA_TOP}/resonance_audio/config/source_config.cc
  ${RA_TOP}/resonance_audio/dsp/biquad_filter.cc
  ${RA_TOP}/resonance_audio/dsp/channel_converter.cc
  ${RA_TOP}/resonance_audio/dsp/circular_buffer.cc
  ${RA_TOP}/resonance_audio/dsp/delay_filter.cc
  ${RA_TOP}/resonance_audio/dsp/distance_attenuation.cc
  ${RA_TOP}/resonance_audio/dsp/fft_manager.cc
  ${RA_TOP}/resonance_audio/dsp/filter_coefficient_generators.cc
  ${RA_TOP}/resonance_audio/dsp/fir_filter.cc
  ${RA_TOP}/resonance_audio/dsp/gain.cc
  ${RA_TOP}/resonance_audio/dsp/gain_mixer.cc
  ${RA_TOP}/resonance_audio/dsp/gain_processor.cc
  ${RA_TOP}/resonance_audio/dsp/mixer.cc
  ${RA_TOP}/resonance_audio/dsp/mono_pole_filter.cc
  ${RA_TOP}/resonance_audio/dsp/multi_channel_iir.cc
  ${RA_TOP}/resonance_audio/dsp/near_field_processor.cc
  ${RA_TOP}/resonance_audio/dsp/occlusion_calculator.cc
  ${RA_TOP}/resonance_audio/dsp/partitioned_fft_filter.cc
  ${RA_TOP}/resonance_audio/dsp/reflections_processor.cc
  ${RA_TOP}/resonance_audio/dsp/resampler.cc
  ${RA_TOP}/resonance_audio/dsp/reverb_onset_compensator.cc
  ${RA_TOP}/resonance_audio/dsp/reverb_onset_update_processor.cc
  ${RA_TOP}/resonance_audio/dsp/sh_hrir_creator.cc
  ${RA_TOP}/resonance_audio/dsp/shoe_box_room.cc
  ${RA_TOP}/resonance_audio/dsp/spectral_reverb.cc
  ${RA_TOP}/resonance_audio/dsp/stereo_panner.cc
  ${RA_TOP}/resonance_audio/dsp/utils.cc
  ${RA_TOP}/resonance_audio/graph/ambisonic_binaural_decoder_node.cc
  ${RA_TOP}/resonance_audio/graph/ambisonic_mixing_encoder_node.cc
  ${RA_TOP}/resonance_audio/graph/binaural_surround_renderer_impl.cc
  ${RA_TOP}/resonance_audio/graph/buffered_source_node.cc
  ${RA_TOP}/resonance_audio/graph/foa_rotator_node.cc
  ${RA_TOP}/resonance_audio/graph/gain_mixer_node.cc
  ${RA_TOP}/resonance_audio/graph/gain_node.cc
  ${RA_TOP}/resonance_audio/graph/graph_manager.cc
  ${RA_TOP}/resonance_audio/graph/hoa_rotator_node.cc
  ${RA_TOP}/resonance_audio/graph/mixer_node.cc
  ${RA_TOP}/resonance_audio/graph/mono_from_soundfield_node.cc
  ${RA_TOP}/resonance_audio/graph/near_field_effect_node.cc
  ${RA_TOP}/resonance_audio/graph/occlusion_node.cc
  ${RA_TOP}/resonance_audio/graph/reflections_node.cc
  ${RA_TOP}/resonance_audio/graph/resonance_audio_api_impl.cc
  ${RA_TOP}/resonance_audio/graph/reverb_node.cc
  ${RA_TOP}/resonance_audio/graph/source_parameters_manager.cc
  ${RA_TOP}/resonance_audio/graph/stereo_mixing_panner_node.cc
  ${RA_TOP}/resonance_audio/node/processing_node.cc
  ${RA_TOP}/resonance_audio/node/sink_node.cc
  ${RA_TOP}/resonance_audio/node/source_node.cc
  ${RA_TOP}/platforms/common/room_effects_utils.cc
  ${RA_TOP}/resonance_audio/utils/buffer_crossfader.cc
  ${RA_TOP}/resonance_audio/utils/buffer_partitioner.cc
  ${RA_TOP}/resonance_audio/utils/buffer_unpartitioner.cc
  ${RA_TOP}/resonance_audio/utils/lockless_task_queue.cc
  ${RA_TOP}/resonance_audio/utils/planar_interleaved_conversion.cc
  ${RA_TOP}/resonance_audio/utils/sample_type_conversion.cc
  ${RA_TOP}/resonance_audio/utils/sum_and_difference_processor.cc
  ${RA_TOP}/resonance_audio/utils/wav.cc
  ${RA_TOP}/resonance_audio/utils/wav_reader.cc
)
add_library(resonance-audio STATIC ${RA_SOURCES} "${QT3P}/pffft/pffft.c"
            "${RA_TOP}/third_party/SADIE_hrtf_database/generated/hrtf_assets.cc")
# EIGEN_MPL2_ONLY makes the build fail if a header under a licence other than MPL-2.0 is included.
target_compile_definitions(resonance-audio PUBLIC EIGEN_MPL2_ONLY)
target_include_directories(resonance-audio SYSTEM PUBLIC "${RA_TOP}" "${RA_TOP}/resonance_audio" "${QT3P}/eigen")
target_include_directories(resonance-audio PRIVATE "${QT3P}/pffft" "${RA_TOP}/third_party/SADIE_hrtf_database/generated")
if(MSVC)
  target_compile_definitions(resonance-audio PUBLIC _USE_MATH_DEFINES NOMINMAX)
  target_compile_options(resonance-audio PRIVATE /W0 /bigobj)
else()
  target_compile_options(resonance-audio PRIVATE -w)
endif()

add_library(simple-binaural-impl STATIC src/plugin.cpp src/binaural-engine.cpp)
target_include_directories(simple-binaural-impl PUBLIC src)
# clapwrapper/vst3.h, for the VST 3 factory information returned in src/plugin.cpp.
target_include_directories(simple-binaural-impl PRIVATE "${clap-wrapper_SOURCE_DIR}/include")
target_compile_definitions(simple-binaural-impl PRIVATE SIMPLE_BINAURAL_VERSION="${PROJECT_VERSION}")
target_link_libraries(simple-binaural-impl PUBLIC clap resonance-audio)

make_clapfirst_plugins(
  TARGET_NAME simple-binaural
  IMPL_TARGET simple-binaural-impl
  OUTPUT_NAME "${PRODUCT_NAME}"
  ENTRY_SOURCE src/plugin-entry.cpp
  BUNDLE_IDENTIFIER "jp.allegromoltov.simple-binaural"
  BUNDLE_VERSION ${PROJECT_VERSION}
  COPY_AFTER_BUILD FALSE
  # The VST 3 validator and the registry both expect the bundle folder layout on Windows.
  WINDOWS_FOLDER_VST3 TRUE
  PLUGIN_FORMATS CLAP VST3
  ASSET_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/assets
)

if(MSVC)
  foreach(module IN ITEMS simple-binaural_clap simple-binaural_vst3)
    target_link_options(${module} PRIVATE /guard:cf /DYNAMICBASE /HIGHENTROPYVA)
  endforeach()
else()
  foreach(module IN ITEMS simple-binaural_clap simple-binaural_vst3)
    # The statically linked C++ runtime keeps the plugins independent of the libstdc++ version of the
    # distribution, and stays hidden. --exclude-libs,ALL would also hide the VST 3 module entry points
    # ModuleEntry and ModuleExit, which the VST 3 SDK defines in a static library.
    target_link_options(${module} PRIVATE -static-libstdc++ -static-libgcc
                        -Wl,--exclude-libs,libstdc++.a:libgcc.a:libgcc_eh.a -Wl,-z,relro,-z,now -Wl,--no-undefined)
  endforeach()
endif()

# Release layout: `cmake --install build --config Release --prefix <dir>`, then zip <dir>.
# The archive carries the license texts of everything linked into the plugins.
# clap-wrapper downloads the SDKs to these places; a layout change in a new clap-wrapper fails here.
set(VST3_SDK_LICENSE "${CMAKE_BINARY_DIR}/cpm/vst3sdk/LICENSE.txt")
set(CLAP_SDK_LICENSE "${clap-wrapper_BINARY_DIR}/cpm/clap/LICENSE")
foreach(license IN ITEMS "${VST3_SDK_LICENSE}" "${CLAP_SDK_LICENSE}")
  if(NOT EXISTS "${license}")
    message(FATAL_ERROR "License file not found: ${license}")
  endif()
endforeach()
# fmt keeps its license in the leading comment of format.h.
file(READ "${clap-wrapper_SOURCE_DIR}/libs/fmt/fmt/format.h" FMT_HEADER)
string(FIND "${FMT_HEADER}" "*/" FMT_COMMENT_END)
string(SUBSTRING "${FMT_HEADER}" 0 ${FMT_COMMENT_END} FMT_LICENSE_TEXT)
string(REGEX REPLACE "^/\\*\n" "" FMT_LICENSE_TEXT "${FMT_LICENSE_TEXT}")
file(WRITE "${CMAKE_BINARY_DIR}/fmt-LICENSE.txt" "${FMT_LICENSE_TEXT}")

# clap-wrapper puts the Windows plugins in per-format folders and the Linux plugins directly in the asset folder.
if(WIN32)
  set(VST3_BUNDLE "${CMAKE_BINARY_DIR}/assets/VST3/${PRODUCT_NAME}.vst3")
else()
  set(VST3_BUNDLE "${CMAKE_BINARY_DIR}/assets/${PRODUCT_NAME}.vst3")
endif()
install(DIRECTORY "${VST3_BUNDLE}" DESTINATION VST3)
install(FILES "$<TARGET_FILE:simple-binaural_clap>" DESTINATION CLAP)
install(FILES LICENSE DESTINATION . RENAME LICENSE.txt)
install(FILES packaging/README.txt DESTINATION .)
install(FILES "${VST3_SDK_LICENSE}" DESTINATION THIRD_PARTY_LICENSES RENAME vst3sdk.txt)
install(FILES "${CLAP_SDK_LICENSE}" DESTINATION THIRD_PARTY_LICENSES RENAME clap.txt)
install(FILES "${clap-wrapper_SOURCE_DIR}/LICENSE" DESTINATION THIRD_PARTY_LICENSES RENAME clap-wrapper.txt)
install(FILES "${CMAKE_BINARY_DIR}/fmt-LICENSE.txt" DESTINATION THIRD_PARTY_LICENSES RENAME fmt.txt)
install(FILES "${RA_TOP}/LICENSE" DESTINATION THIRD_PARTY_LICENSES RENAME resonance-audio.txt)
install(FILES "${RA_TOP}/third_party/SADIE_hrtf_database/LICENSE" DESTINATION THIRD_PARTY_LICENSES
        RENAME sadie-hrtf-database.txt)
install(FILES "${QT3P}/pffft/LICENSE" DESTINATION THIRD_PARTY_LICENSES RENAME pffft.txt)
install(FILES "${QT3P}/eigen/COPYING.MPL2" DESTINATION THIRD_PARTY_LICENSES RENAME eigen-MPL2.txt)

include(CTest)
if(BUILD_TESTING)
  add_executable(binaural-test tests/binaural-test.cpp src/binaural-engine.cpp)
  target_include_directories(binaural-test PRIVATE src)
  target_link_libraries(binaural-test PRIVATE resonance-audio)
  add_test(NAME binaural COMMAND binaural-test)
  # The sanitizers cover the unit test only. clap-wrapper links the VST 3 module with --no-undefined,
  # which the sanitizer runtimes do not satisfy.
  if(SBA_SANITIZE AND NOT MSVC)
    set(sanitize_flags -fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer)
    target_compile_options(binaural-test PRIVATE ${sanitize_flags})
    target_link_options(binaural-test PRIVATE -fsanitize=address,undefined)
  endif()
endif()
