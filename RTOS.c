#include <stdio.h>
#include <string.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <matrix.h>
#include <signal.h>

int __algorithm;

int parameterAlgorithm(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Warning!, only one input parameter must be provided\n");
        return 0;
    }

    if (!strcmp(argv[1], "RM"))
    {
        printf("RM\n");
        __algorithm = 0;
        return 1;
    }
    else if (!strcmp(argv[1], "EDF"))
    {
        __algorithm = 1;
        printf("EDF\n");
        return 1;
    }

    else
    {
        printf("Algorithm error\n");
        return 0;
    }
}

int createProcess(void)
{
    if (fork() == 0)
    {
        int child_id = getpid();
        printf("\n[son] pid %d from [parent] pid %d\n", child_id, getppid());
       /*  wait(NULL); */ 
        return child_id;
    }
    return -1;
}

void kill_child(int child_pid)
{
    kill(child_pid, SIGKILL);
}

int main(int argc, char *argv[])
{
    if (!parameterAlgorithm(argc, argv))
    {
        return 0;
    }

    /* for (int c = 0; c < M_ROW; c++)
    {
        for (int d = 0; d < M_COL; d++)
        {
            printf("%d\t", MAZE_MATRIX[c][d]);
        }
        printf("\n");
    } */
    // return 0;

/*     int p1 = createProcess();
    int p2 = createProcess();
    int p3 = createProcess();


    if (p1 == -1 || p2 == -1)
    {
        return 0;
    } */
    

    if (!al_init())
    {
        printf("couldn't initialize allegro\n");
        return 1;
    }

    if (!al_install_keyboard())
    {
        printf("couldn't initialize keyboard\n");
        return 1;
    }

    if (!al_init_image_addon())
    {
        printf("couldn't initialize image addon\n");
        return 1;
    }

    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 30.0);
    if (!timer)
    {
        printf("couldn't initialize timer\n");
        return 1;
    }

    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    if (!queue)
    {
        printf("couldn't initialize queue\n");
        return 1;
    }

    ALLEGRO_DISPLAY *disp = al_create_display(608, 608);
    if (!disp)
    {
        printf("couldn't initialize display\n");
        return 1;
    }

    ALLEGRO_FONT *font = al_create_builtin_font();
    if (!font)
    {
        printf("couldn't initialize font\n");
        return 1;
    }

    ALLEGRO_BITMAP *alien = al_load_bitmap("../src/alien.png");
    ALLEGRO_BITMAP *maze = al_load_bitmap("../src/maze.png");

    if (!alien || !maze)
    {
        puts("couldn't load alien\n");
        return 1;
    }

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));

    bool done = false;
    bool redraw = true;
    ALLEGRO_EVENT event;

    int x_alien = 0;

    al_start_timer(timer);
    while (1)
    {
        al_wait_for_event(queue, &event);

        switch (event.type)
        {
        case ALLEGRO_EVENT_TIMER:

            // game logic goes here.
            if (x_alien >= 0 && x_alien < 608)
            {
                x_alien++;
            }
            else
            {
                x_alien = 0;
            }

            redraw = true;
            break;

        case ALLEGRO_EVENT_KEY_DOWN:
            if (event.keyboard.keycode == ALLEGRO_KEY_X)
                done = true;
            break;
        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            done = true;
            break;
        }

        if (done)
            break;

        if (redraw && al_is_event_queue_empty(queue))
        {
            al_clear_to_color(al_map_rgb(255, 255, 255));

            al_draw_bitmap(maze, 0, 0, 0);

            al_draw_bitmap(alien, x_alien, 32 * 7, 0);

            al_flip_display();

            redraw = false;
        }
    }

    al_destroy_bitmap(alien);
    al_destroy_bitmap(maze);

    al_destroy_font(font);
    al_destroy_display(disp);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);

    return 0;
}