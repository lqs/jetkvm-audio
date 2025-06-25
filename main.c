#include <alsa/asoundlib.h>
#include <opus.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SAMPLE_RATE 48000
#define CHANNELS 2
#define FRAME_SIZE 960  // 20ms for 48kHz
#define PCM_BUFFER_SIZE (FRAME_SIZE * CHANNELS)
#define MAX_PACKET_SIZE 1500
#define UNIX_SOCKET_PATH "/var/run/jetkvm_audio.sock"

int main() {
    // ALSA init
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;
    if (snd_pcm_open(&pcm_handle, "hw:1,0", SND_PCM_STREAM_CAPTURE, 0) < 0)
    {
        fprintf(stderr, "Failed to open PCM device\n");
        return 1;
    }
    snd_pcm_hw_params_malloc(&params);
    snd_pcm_hw_params_any(pcm_handle, params);
    snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(pcm_handle, params, CHANNELS);
    snd_pcm_hw_params_set_rate(pcm_handle, params, SAMPLE_RATE, 0);
    snd_pcm_hw_params_set_period_size(pcm_handle, params, FRAME_SIZE, 0);
    snd_pcm_hw_params(pcm_handle, params);
    snd_pcm_hw_params_free(params);

    snd_pcm_prepare(pcm_handle);

    // Opus encoder setup
    int err;
    OpusEncoder *encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, OPUS_APPLICATION_AUDIO, &err);
    opus_encoder_ctl(encoder, OPUS_SET_BITRATE(64000));
    opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(5));
    unsigned char opus_data[MAX_PACKET_SIZE];

    // Unix socket setup
    int sockfd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }
    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, UNIX_SOCKET_PATH);

    fprintf(stderr, "Connecting to %s\n", UNIX_SOCKET_PATH);
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("connect");
        close(sockfd);
        return 1;
    }
    fprintf(stderr, "Connected to %s\n", UNIX_SOCKET_PATH);

    while (1)
    {
        short pcm_buffer[PCM_BUFFER_SIZE];
        int pcm_rc = snd_pcm_readi(pcm_handle, pcm_buffer, FRAME_SIZE);
        if (pcm_rc < 0)
        {
            fprintf(stderr, "Error reading PCM data: %s\n", snd_strerror(pcm_rc));
            continue;
        }
        int nb_bytes = opus_encode(encoder, pcm_buffer, FRAME_SIZE, opus_data, MAX_PACKET_SIZE);
        printf("Encoded %d -> %d bytes\n", FRAME_SIZE * CHANNELS * sizeof(short), nb_bytes);
        if (nb_bytes > 0)
        {
            if (sendto(sockfd, opus_data, nb_bytes, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
                perror("sendto");
                continue;
            }
        }
    }
}
