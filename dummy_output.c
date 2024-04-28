#include <retrovert/log.h>
#include <retrovert/output.h>
#include <retrovert/settings.h>
#include <tinycthread.h>
#include <stdlib.h>

#define PLUGIN_NAME "DummyAudio"

const RVLog* g_rv_log = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct DummyOutput {
    thrd_t thread;
    RVPlaybackCallback callback;
    int running;
    int sample_rate;
    int channels;
    int format;
} DummyOutput;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* dummyaudio_output_create(const RVService* services) {
    DummyOutput* data = calloc(1, sizeof(DummyOutput));
    return data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int dummyaudio_destroy(void* user_data) {
    DummyOutput* data = (DummyOutput*)user_data;
    free(data);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void dummyaudio_data_callback(DummyOutput* callback_data, void* output, int frame_count) {
    DummyOutput* data = (DummyOutput*)callback_data;
    RVAudioFormat format = { data->format, data->channels, data->sample_rate };
    data->callback.callback(data->callback.user_data, output, format, frame_count);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int thread_callback(void* arg) {
    DummyOutput* data = (DummyOutput*)arg;
    uint8_t* output_buffer = (uint8_t*)malloc(480 * data->channels * sizeof(float)); // 480 samples of n channels * sizeof float
    
    struct timespec sleep_time = { 0, 10000000 }; // sleep 0.01 sec (10 ms) between each request

    while (data->running) {
        dummyaudio_data_callback(data, output_buffer, 480);
        if (thrd_sleep(&sleep_time, NULL)) {
            rv_error("Unable to sleep worker thread");
        }
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void dummyaudio_start(void* user_data, RVPlaybackCallback* callback) {
    int error = 0;
    DummyOutput* data = (DummyOutput*)user_data;
    data->callback = *callback;
    data->sample_rate = 48000;
    data->format = RVAudioStreamFormat_F32;
    data->running = 1;
    data->channels = 2;

    if (thrd_create(&data->thread, thread_callback, data) != thrd_success) {
        rv_error("Unable to create thread");
        return;
    }

    rv_info("dummyaudio device created");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void dummyaudio_stop(void* user_data) {
    DummyOutput* data = (DummyOutput*)user_data;
    data->running = 0;
    thrd_detach(data->thread);
}

static const char* s_targets[] = { "dummy audio" };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RVOutputTargets dummyaudio_output_targets_info(void*) {
    return (RVOutputTargets){ s_targets, 1 };
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void dummyaudio_static_init(const RVService* service_api) {
    g_rv_log = RVService_get_log(service_api, RV_LOG_API_VERSION);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static RVOutputPlugin s_dummyaudio_plugin = {
    RV_OUTPUT_PLUGIN_API_VERSION,
    PLUGIN_NAME,
    "0.0.1",
    NULL,
    dummyaudio_output_create,
    dummyaudio_destroy,
    dummyaudio_output_targets_info,
    dummyaudio_start,
    dummyaudio_stop,
    dummyaudio_static_init,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern RV_EXPORT RVOutputPlugin* rv_output_plugin() {
    return &s_dummyaudio_plugin;
}

