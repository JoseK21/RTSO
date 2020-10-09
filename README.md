### Instalation
    sudo add-apt-repository ppa:allegro/5.2
    sudo apt-get install liballegro*5.2 liballegro*5-dev

### Intrucctions
    make
    or
    gcc hello.c -o hello $(pkg-config allegro-5 allegro_font-5 --libs --cflags)
    gcc v2.c -o v2 $(pkg-config allegro-5 allegro_font-5 allegro_image-5  --libs --cflags)

### Crate Maze
    https://www.pixilart.com/draw