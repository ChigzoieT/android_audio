#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "jni.h"
#include "aaudio/AAudio.h"
#include "messages.h"
#include "string.h"
#include "limits.h"
#include "unistd.h"



#pragma clang diagnostic push
#pragma ide diagnostic ignored "ArrayIndexOutOfBounds"
#define MAXVAL INT32_MAX
#define MINVAL INT32_MIN
#define buffed 40

pid_t pid;


int32_t hellome[41];



void callbackforerror(AAudioStream *stream, void *userdata, aaudio_result_t error){
    fprintf(stderr, "Error detected: %i\n", error);
}



AAudioStream *stream = NULL;
AAudioStreamBuilder *builder;

void writetofile(int audiodata){
    const char* pathname ="data/data/com.aud.audiodata/files/audio.txt";

    FILE *fp;
    fp = fopen(pathname, "w");

    if(fp == NULL){
        LOGE("Error in opening file");
    }

    else{
        LOGI("file opened successfully!");
    }
    fprintf(fp, "%d", audiodata);
}




JNIEXPORT void JNICALL
Java_com_aud_audiodata_MainActivity_inputaudiostreaming(JNIEnv *env, jobject thiz) {



    aaudio_result_t result = AAudio_createStreamBuilder(&builder);
    if(result!= AAUDIO_OK){
        LOGE("Error: %d", result);
    }


    int deviceid = AAUDIO_UNSPECIFIED;
    aaudio_direction_t direction = AAUDIO_DIRECTION_INPUT;
    aaudio_sharing_mode_t sharingMode = AAUDIO_SHARING_MODE_SHARED;
    int32_t samplerate = 44100;
    int32_t channelcount = AAUDIO_CHANNEL_MONO;
    aaudio_format_t  aaudioFormat = AAUDIO_FORMAT_PCM_I16;
    int32_t frames = 1024;
    typedef int32_t sample_type;


    AAudioStreamBuilder_setDeviceId(builder, deviceid);
    AAudioStreamBuilder_setDirection(builder, direction);
    AAudioStreamBuilder_setSharingMode(builder, sharingMode);
    AAudioStreamBuilder_setSampleRate(builder, samplerate);
    AAudioStreamBuilder_setChannelCount(builder, channelcount);
    AAudioStreamBuilder_setFormat(builder, aaudioFormat);
    AAudioStreamBuilder_setBufferCapacityInFrames(builder, frames);
    AAudioStreamBuilder_setPerformanceMode(builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);




    AAudioStreamBuilder_setErrorCallback(builder, callbackforerror, NULL);




    result = AAudioStreamBuilder_openStream(builder, &stream);
    if(result != AAUDIO_OK){
        LOGE("issue opening stream");
    }else{
        LOGI("stream opening successfully");
    }


    int32_t timeout = 5;

    result = AAudioStream_requestStart(stream);
    if(result != AAUDIO_OK){
        LOGE("Error starting audio stream: %s", AAudio_convertResultToText(result));
        exit(-1);
    } else{
        LOGI("stream request successful");
    }

    channelcount = (int32_t)AAudioStream_getChannelCount(stream);

    LOGI("channel count is: %d", channelcount);

    int32_t samplePerFrame = channelcount;


    int32_t* audioData = (int32_t *)  malloc(sizeof(int32_t) * frames * samplerate);



    int fds[2];
    pipe(fds);
    int p;

    char buf[30];
#define BUFFER 30

    pid = fork();
    if(pid == 0){
        for(int i = 0; i<=BUFFER; i++){

            result = AAudioStream_read(stream, audioData, frames, timeout);
            if (result < 0) {
                LOGE("No result found");
            }else{
                LOGI("result found %d", result);
            }
            write(fds[1], &audioData, sizeof(audioData));
            /*writetofile((int) audioData);*/
        }
    } else{
       for(int i=0; i<=100; i++){
            read(fds[0], hellome, sizeof(hellome));
           LOGI("%d", hellome);
           hellome[i] = p;
           /*writetofile((int)p);*/
        }
        LOGI("audio started...");
    }

    if (result != frames) {

        // Pad the buffer with zeros
        memset( (sample_type*)audioData + result * samplePerFrame, 0,
                sizeof(sample_type) * (frames - result) * samplePerFrame);
    }
    AAudioStreamBuilder_delete(builder);









    /*  AAudioStream_requestStop(stream);
    AAudioStream_close(stream);

    // Clean up AAudio
    AAudio_terminate();

    return 0;*/


}

JNIEXPORT void JNICALL
Java_com_aud_audiodata_MainActivity_closestream(JNIEnv *env, jobject thiz) {



    if (stream!=NULL){
        AAudioStream_requestStop(stream);
        AAudioStream_close(stream);
    }

}


JNIEXPORT jstring JNICALL
Java_com_aud_audiodata_MainActivity_getpcmdata(JNIEnv *env, jobject thiz) {
    const char* pathname ="data/data/com.aud.audiodata/files/audio.txt";

    FILE *fp;
    fp = fopen(pathname, "r");

    if(fp == NULL){
        LOGE("Error in opening file");
    }else{
        LOGI("file opened for reading..");
    }
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    char hello = (char )fileSize;
    char* bypass = (char *)malloc(sizeof(hello));
    bypass[0] = hello;
    /*fseek(fp, 0, SEEK_SET);

    char *content = (char*) malloc(fileSize+1);

    if(content==NULL){
        LOGE("Memory allocation error");
        fclose(fp);
        return NULL;
    }

    if (fread(content, 1, fileSize, fp) !=fileSize){
        LOGI("Error reading file");
        free(content);
        fclose(fp);
        return 0;
    }

    content[fileSize] = '\0';*/
    fclose(fp);


    return (*env)->NewStringUTF(env, hellome);

}
#pragma clang diagnostic pop