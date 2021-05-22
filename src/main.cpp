#include <stdio.h>
#include <errno.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// https://github.com/ocornut/imgui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_stdlib.h"

// https://github.com/nothings/stb/blob/master/stb_image.h
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"


//#define IMAGES_DIR     "./images/"
#define FONT_FILE      "Topaz-8.ttf"
#define FONT_SCALE     16.0

#define COLOR_RGB(r,g,b) ImVec4(r/255.0, g/255.0, b/255.0, 1.0)


#include "synth.hpp"


static int window_width = 0;
static int window_height = 0;

/*
u32 load_texture(char* path) {
	u32 tex = 0;
	int width = 0;
	int height = 0;
	int num_channel = 0;
	
	u8* tex_data = stbi_load(path, &width, &height, &num_channel, 0);
	if(tex_data != NULL) {
		glGenTextures(1, &tex);

		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

		int format = 0;
		switch(num_channel) {
			case 1: format = GL_RED; break;
			case 2: format = GL_RG;  break;
			case 3: format = GL_RGB; break;
			case 4: format = GL_RGBA; break;
			default:
				fprintf(stderr, "No valid format found for %i while creating texture from \"%s\"\n", 
						num_channel, path);
				break;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, tex_data);
		stbi_image_free(tex_data);

	}
	else {
		fprintf(stderr, "Failed to load data from \"%s\". %s\n", path, stbi_failure_reason());
	}

	return tex;
}

void unload_texture(u32* tex) {
	glDeleteTextures(1, tex);
}
*/

void glfw_window_size_callback(GLFWwindow* window, int w, int h) {
	glViewport(0, 0, w, h);
	window_width = w;
	window_height = h;
}


void sequencer_callback(struct seq_t* seq) {
}


int main() {

	if(SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL2\n%s\n", SDL_GetError());
		return -1;
	}

	synth_init();
	synth_set_seq_callback(sequencer_callback);


	if(!glfwInit()) {
		fprintf(stderr, "Failed to initialize glfw!\n");
		return -1;
	}
	
	glfwWindowHint(GLFW_MAXIMIZED, 1);
	glfwWindowHint(GLFW_FOCUS_ON_SHOW, 1);
	glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

	GLFWwindow* window = glfwCreateWindow(1000, 800, "uwu", NULL, NULL);
	if(window == NULL) {
		fprintf(stderr, "Failed to create window\n");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glfwSetWindowSizeCallback(window, glfw_window_size_callback);

	if(glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize glew!\n");
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, 1);
	ImGui_ImplOpenGL3_Init("#version 330");

	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle& s = ImGui::GetStyle();
	io.Fonts->AddFontFromFileTTF(FONT_FILE, FONT_SCALE);

	s.WindowRounding = 0.0;
	s.FrameRounding = 2.35;
	s.GrabRounding = 3.0;
	s.GrabMinSize = 15.0;
	s.ItemSpacing = ImVec2(5, 3);
	s.WindowPadding = ImVec2(20, 10);

	s.Colors[ImGuiCol_Text]      = COLOR_RGB(200, 210, 210);
	s.Colors[ImGuiCol_WindowBg]  = COLOR_RGB(25, 25, 25);
	s.Colors[ImGuiCol_FrameBg]   = COLOR_RGB(40, 40, 40);
	s.Colors[ImGuiCol_FrameBgHovered]  = COLOR_RGB(50, 50, 50);
	s.Colors[ImGuiCol_FrameBgActive]   = COLOR_RGB(50, 50, 50);

	s.Colors[ImGuiCol_Border]      = COLOR_RGB(30, 70, 70);
	s.Colors[ImGuiCol_CheckMark]   = COLOR_RGB(30, 255, 255);
	s.Colors[ImGuiCol_ChildBg]     = COLOR_RGB(20, 20, 20);	
	s.Colors[ImGuiCol_SliderGrab]        = COLOR_RGB(70, 135, 140);
	s.Colors[ImGuiCol_SliderGrabActive]  = COLOR_RGB(30, 255, 255);


	while(!glfwWindowShouldClose(window)) {
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(window_width, window_height));
		ImGui::Begin("window", NULL, 
				  ImGuiWindowFlags_NoTitleBar
			   	| ImGuiWindowFlags_NoResize
				| ImGuiWindowFlags_NoCollapse
				| ImGuiWindowFlags_NoSavedSettings
				| ImGuiWindowFlags_NoMove);

		s.Colors[ImGuiCol_FrameBg]         = COLOR_RGB(40, 40, 40);
		s.Colors[ImGuiCol_FrameBgHovered]  = COLOR_RGB(50, 50, 50);
		s.Colors[ImGuiCol_FrameBgActive]   = COLOR_RGB(50, 50, 50);
		s.Colors[ImGuiCol_SliderGrab]        = COLOR_RGB(70, 135, 140);
		s.Colors[ImGuiCol_SliderGrabActive]  = COLOR_RGB(30, 255, 255);
		
		struct seq_t* seq = synth_seq();

		ImGui::SetNextItemWidth(450);
		ImGui::SliderFloat("##VOLUME", synth_master_vol(), 0.0, 1.0, "Master volume: %1.3f");
		if(ImGui::Checkbox("Sequencer", (bool*)&seq->enabled)) {
			seq->current = 0;
		}
		
		ImGui::SetNextItemWidth(450);
		ImGui::SliderFloat("##NOTETIME", &seq->note_time, 0.0, 10.0, "Note time: %1.3f");
		
		ImGui::SetNextItemWidth(450);
		ImGui::SliderInt("##TEMPO", &seq->tempo, 20, 200, "Tempo: %i");

		for(int i = 0; i < SYNTH_SEQ_PATTERN_LENGTH; i++) {
			s.Colors[ImGuiCol_SliderGrab] = (i == seq->current) ? COLOR_RGB(30, 130, 30) : COLOR_RGB(80, 80, 80);
			ImGui::PushID(i);
			ImGui::VSliderInt("##", ImVec2(40, 100), &seq->pattern[i], -1, 24);
			ImGui::PopID();
			if(i+1 < SYNTH_SEQ_PATTERN_LENGTH) {
				ImGui::SameLine();
			}
		}

		{
			s.Colors[ImGuiCol_FrameBg]         = COLOR_RGB(80, 40, 80);
			s.Colors[ImGuiCol_FrameBgHovered]  = COLOR_RGB(90, 50, 90);
			s.Colors[ImGuiCol_FrameBgActive]   = COLOR_RGB(90, 50, 90);
			s.Colors[ImGuiCol_SliderGrab]        = COLOR_RGB(200, 35, 200);
			s.Colors[ImGuiCol_SliderGrabActive]  = COLOR_RGB(200, 35, 200);
			
			ImGui::BeginChild("##LOWFREQOSCILLATORS", ImVec2(500, 800), 1);
	
			
			for(int i = 0; i < SYNTH_NUM_LFO; i++) {
				struct lfo_t* lfo = synth_lfo(i);
				ImGui::Text("LFO(%i)", i);
				ImGui::PushID(i);
				ImGui::SliderFloat("##", &lfo->freq, 0.0, 10.0, "Frequency: %1.3f");
				ImGui::PopID();
				ImGui::PushID(i+SYNTH_NUM_LFO);
				ImGui::SliderFloat("##", &lfo->ampl, 0.0, 10.0, "Amplitude: %1.3f");
				ImGui::PopID();
				ImGui::Separator();
				ImGui::Spacing();

			}
			
			ImGui::EndChild();
		}
		
		ImGui::SameLine();

		{
			s.Colors[ImGuiCol_FrameBg]         = COLOR_RGB(10, 60, 80);
			s.Colors[ImGuiCol_FrameBgHovered]  = COLOR_RGB(20, 70, 90);
			s.Colors[ImGuiCol_FrameBgActive]   = COLOR_RGB(20, 70, 90);
			s.Colors[ImGuiCol_SliderGrab]        = COLOR_RGB(35, 130, 200);
			s.Colors[ImGuiCol_SliderGrabActive]  = COLOR_RGB(35, 130, 200);
			
			ImGui::BeginChild("##ENVELOPES", ImVec2(500, 800), 1);
			for(int i = 0; i < SYNTH_NUM_ENV; i++) {
				struct env_t* env = synth_env(i);
				ImGui::Text("ENVELOPE(%i)", i);
				ImGui::PushID(i);
				ImGui::SliderFloat("##", &env->attack, 0.001, 10.0, "Attack: %1.3f");
				ImGui::PopID();
				ImGui::PushID(i+SYNTH_NUM_ENV);
				ImGui::SliderFloat("##", &env->decay, 0.0, 10.0, "Decay: %1.3f");
				ImGui::PopID();
				ImGui::PushID(i+SYNTH_NUM_ENV*2);
				ImGui::SliderFloat("##", &env->sustain, 0.0, 10.0, "Sustain: %1.3f");
				ImGui::PopID();
				ImGui::PushID(i+SYNTH_NUM_ENV*3);
				ImGui::SliderFloat("##", &env->release, 0.0, 10.0, "Release: %1.3f");
				ImGui::PopID();
				ImGui::Separator();
				ImGui::Spacing();
			}


			ImGui::EndChild();
		}
			
		ImGui::SameLine();

		{
			s.Colors[ImGuiCol_FrameBg]         = COLOR_RGB(120, 70, 20);
			s.Colors[ImGuiCol_FrameBgHovered]  = COLOR_RGB(130, 80, 30);
			s.Colors[ImGuiCol_FrameBgActive]   = COLOR_RGB(130, 80, 30);
			s.Colors[ImGuiCol_SliderGrab]        = COLOR_RGB(250, 120, 50);
			s.Colors[ImGuiCol_SliderGrabActive]  = COLOR_RGB(250, 120, 50);
			
			ImGui::BeginChild("##OSCILLATORS", ImVec2(500, 800), 1);
			
			for(int i = 0; i < SYNTH_NUM_OSC; i++) {
				struct osc_t* osc = synth_osc(i);
				ImGui::Text("OSCILLATOR(%i)", i);
				ImGui::PushID(i);
				ImGui::SliderFloat("##", &osc->vol, 0.0, 1.0, "Volume: %1.3f");
				ImGui::PushID(i+SYNTH_NUM_OSC);
				ImGui::SliderFloat("##", &osc->note_offset, -24.0, 24.0, "Note offset: %1.3f");
				ImGui::PopID();
				ImGui::PushID(i+SYNTH_NUM_OSC*2);
				ImGui::SliderInt("##", &osc->wave_type, 0, 4, "Shape: %i");
				ImGui::PopID();
				ImGui::Separator();
				ImGui::Spacing();
			}
			
			ImGui::EndChild();
		}

		ImGui::End();
		
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}


