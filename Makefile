
FLD: src/FLD.cpp 
	g++ -g -Wall src/FLD.cpp -o bin/FLD -O2 -ldl `pkg-config --cflags --libs opencv4`

undulate: src/undulate.cpp lib/imgtools.a
	g++ -g -o bin/undulate external/glad.c src/undulate.cpp lib/imgtools.a -O2 -ldl -lglfw `pkg-config --cflags --libs opencv4`

cannyvideo: src/cannyvideo.cpp src/imgtools.cpp
	g++ -g -o bin/cannyvideo src/cannyvideo.cpp src/imgtools.cpp  -O2 `pkg-config --cflags --libs opencv4`

extract_lighting: src/extract_lighting.cpp
	g++ -g -o bin/extract_lighting src/extract_lighting.cpp -O2 -ldl `pkg-config --cflags --libs opencv4`

generate_orientation: src/generate_orientation_bias.cpp
	g++ -g -o bin/generate_orientation src/generate_orientation_bias.cpp -O2 -ldl `pkg-config --cflags --libs opencv4`

derivatives: src/derivatives.cpp
	g++ -g -o bin/derivatives src/derivatives.cpp -O2 -ldl `pkg-config --cflags --libs opencv4`

deepdreamSurface: deepdreamSurface.cpp
	g++ -g -o bin/deepdreamSurface src/deepdreamSurface.cpp -O2 -ldl `pkg-config --cflags --libs opencv4`

build/render_ffmpeg.o: src/render_ffmpeg.cpp
	g++ -c -o build/render_ffmpeg.o src/render_ffmpeg.cpp -lavcodec -lswscale -lavutil -lavformat `pkg-config --cflags --libs opencv4`

gui: src/gui.cpp build/render_ffmpeg.o lib/imgtools.a
	g++ -g -std=c++17 -o bin/gui src/gui.cpp lib/imgtools.a build/render_ffmpeg.o -O2 `pkg-config --cflags eigen3` -lnanogui -ldl -lGL -lavcodec -lswscale -lavutil -lavformat `pkg-config --cflags --libs opencv4`

lib/imgtools.a: build/imgtools.o
	ar rvs lib/imgtools.a build/imgtools.o

build/imgtools.o: src/imgtools.cpp src/imgtools.h
	g++ -c src/imgtools.cpp -o build/imgtools.o -Wall `pkg-config --cflags opencv4`
