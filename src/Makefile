all:
	gcc -Wall -g -I"../raylib" map_editor.c engine/texture_manager.c config/config.c config/cJSON.c -L"../raylib" -lraylib -lm -lpthread -ldl -lrt -lX11 -o map_editor
test:
	gcc -Wall -Wextra -g map_editor.c -DTEST_BUILD ../src/engine/engine.c ../src/engine/engine_context.c ../src/menu.c ../src/engine/texture_manager.c ../src/config/config.c ../src/config/cJSON.c ../src/door.c ../src/engine/console.c ../src/map.c ../src/player.c  -o test_editor `pkg-config --cflags --libs check` -lm -Wall -g -I"../raylib" -L"../raylib" -lraylib -lm -lpthread -ldl -lrt -lX11 -I"../net-prog/paho.mqtt.c/src" -L"../net-prog/paho.mqtt.c/src" -l:libpaho-mqtt3a.a
