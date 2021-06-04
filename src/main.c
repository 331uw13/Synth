#include <stdio.h>

#include <alsa/asoundlib.h>
#include <pthread.h>

#include "synth.h"
#include "util.h"


void die(u32 pcm) {
	fprintf(stderr, "%s\n", snd_strerror(pcm));
	exit(-1);
}


#define TO2PI(a) ((a)*M_PI*2.0)

snd_pcm_uframes_t buffer_size;
snd_pcm_uframes_t period_size;
snd_pcm_t* pcm_handle = NULL;
short* audio_buffer = NULL;

double a_time = 0.0;
double a_pos = 0.0;


void* audio_thread() {
	while(1) {
		for(u32 i = 0; i < buffer_size; i++) {
		
			double o = sin(TO2PI(130.0)*a_time);
			audio_buffer[i] = (short)(o*8000.0);

			a_pos += 1.0;
			a_time = a_pos/44100.0;
		}

		snd_pcm_writei(pcm_handle, audio_buffer, buffer_size);
	}

	pthread_exit(0);
}


void main_loop(struct state_t* s) {
	glClearColor(0.08, 0.08, 0.08, 1.0);


	int test = 0;
	double test_d = 0.0;


	int pcm = 0;
	u32 tmp = 0;
	u32 rate = 44100;

	snd_pcm_hw_params_t*  params = NULL;


	if((pcm = snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		die(pcm);
	}

	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(pcm_handle, params);


	if((pcm = snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		die(pcm);
	}

	if((pcm = snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S16_LE)) < 0) {
		die(pcm);
	}

	if((pcm = snd_pcm_hw_params_set_channels(pcm_handle, params, 1)) < 0) {
		die(pcm);
	}

	buffer_size = 4096;
	period_size = 512;

	if((pcm = snd_pcm_hw_params_set_rate_near(pcm_handle, params, &rate, 0)) < 0) {
		die(pcm);
	}

	if((pcm = snd_pcm_hw_params_set_buffer_size_near(pcm_handle, params, &buffer_size)) < 0) {
		die(pcm);
	}
	
	if((pcm = snd_pcm_hw_params_set_period_size_near(pcm_handle, params, &period_size, 0)) < 0) {
		die(pcm);
	}



	if((pcm = snd_pcm_hw_params(pcm_handle, params)) < 0) {
		die(pcm);
	}

	printf("pcm name = %s\n", snd_pcm_name(pcm_handle));
	printf("pcm state = %s\n", snd_pcm_state_name(snd_pcm_state(pcm_handle)));

	snd_pcm_hw_params_get_rate(params, &tmp, 0);
	printf("%i\n", tmp);

	printf("buffer_size = %li\n", buffer_size);
	printf("period_size = %li\n", period_size);

	audio_buffer = malloc(sizeof *audio_buffer * buffer_size);


	printf("pcm name = %s\n", snd_pcm_name(pcm_handle));
	printf("pcm state = %s\n", snd_pcm_state_name(snd_pcm_state(pcm_handle)));

	pthread_t thread;
	pthread_create(&thread, NULL, audio_thread, NULL);


	while(!glfwWindowShouldClose(s->w) && !(s->flags & SHOULD_QUIT)) {
		s->gui->flags &= ~GGUI_MOUSE_DOWN;
		
		glfwWaitEvents();		
		glClear(GL_COLOR_BUFFER_BIT);

		ggui_checkbox(s->gui, 100.0, 100.0, &test);
		ggui_knob(s->gui, 180.0, 200.0, &test_d, 0.0, 5.0);

		glfwSwapBuffers(s->w);
	}


	free(audio_buffer);
	snd_pcm_drop(pcm_handle);
	snd_pcm_drain(pcm_handle);
	snd_pcm_close(pcm_handle);
	snd_pcm_hw_free(pcm_handle);
	puts("exit.");
}


int main() {
	struct state_t* s = NULL;
	s = synth_init();

	if(s != NULL && !(s->flags & NOT_INITIALIZED)) {
		main_loop(s);
	}

	synth_quit(s);
	return 0;
}




