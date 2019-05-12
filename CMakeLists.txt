cmake_minimum_required(VERSION 3.14)
project(CLion)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-lopus")

add_executable(CLion AudioGrabberALSA.cpp AudioGrabberALSA.h AudioSignalParams.h AudioFrame.h AudioReplayALSA.cpp AudioReplayALSA.h AudioTest.cpp IAudioFrameProducer.h AudioEncoderOpus.cpp AudioEncoderOpus.h EncodedAudio.h IAudioEncoded.h opus/opus.h opus/opus_types.h opus/opus_defines.h AudioDecoderOpus.cpp AudioDecoderOpus.h IAudioDecoded.h Client.cpp Client.h IClientCallback.h spdlog/spdlog.h Log.cpp Log.h)

find_package(ALSA REQUIRED)
if (ALSA_FOUND)
    include_directories(${ALSA_INCLUDE_DIRS})
    target_link_libraries (CLion ${ALSA_LIBRARIES})
endif(ALSA_FOUND)

find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
    target_compile_options(CLion PUBLIC "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
    target_link_libraries(CLion "${CMAKE_THREAD_LIBS_INIT}")
endif()

target_link_libraries(CLion opus)


set(SPDLOG_DIR ${CMAKE_SOURCE_DIR})
include_directories(${SPDLOG_DIR})