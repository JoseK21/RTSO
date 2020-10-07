#include <stdio.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>

int main()
{
    al_init();
    al_install_keyboard();

    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 30.0);
    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    ALLEGRO_DISPLAY *disp = al_create_display(1200, 600);
    ALLEGRO_FONT *font = al_create_builtin_font();

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));

    bool redraw = true;
    ALLEGRO_EVENT event;

    if (!al_init_image_addon())
    {
        printf("couldn't initialize image addon\n");
        return 1;
    }
    ALLEGRO_BITMAP *mysha = al_load_bitmap("mysha.png");
    if (!mysha)
    {
        puts("couldn't load mysha\n");
        return 1;
    }

    int x_mysha = 0;
    al_start_timer(timer);
    while (1)
    {
        al_wait_for_event(queue, &event);

        if (event.type == ALLEGRO_EVENT_TIMER)
            redraw = true;
        else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            break;
        else if ((event.type == ALLEGRO_EVENT_KEY_DOWN))
            if (event.keyboard.keycode == ALLEGRO_KEY_X)
                break;

        if (redraw && al_is_event_queue_empty(queue))
        {
            al_clear_to_color(al_map_rgb(0, 0, 0));                                 // Clears the screen to black
            al_draw_text(font, al_map_rgb(255, 255, 255), 0, 0, 0, "Hello world!"); // Draws Hello world! in white to the top-left corner of the window

            al_draw_bitmap(mysha, 100, 100, 0);

            al_flip_display();                                                      // Commits the result

            redraw = false; // flag to reset and draw the next frame.
        }
    }

    // Shutdown
    al_destroy_bitmap(mysha);
    al_destroy_font(font);
    al_destroy_display(disp);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);

    return 0;
}