// https://www.youtube.com/watch?v=kT-Mz87-HcQ
// ppm 6

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <time.h>
#include <math.h>

#define WIDTH 800
#define HEIGHT 600 
typedef uint32_t Image[HEIGHT][WIDTH];
typedef uint32_t color_t;

//#define CENTROID_NUM (rand() % 40) + 5
#define CENTROID_NUM 20

// little endian hex colors
// alpha blue green red
// alpha is opacity
#define COLOR_RED    0xFF0000FF
#define COLOR_GREEN  0xFF00FF00
#define COLOR_BLUE   0xFFFF0000
#define COLOR_BLACK  0xFF000000
#define COLOR_PASTEL 0xFFF2DDF3

typedef struct Point {
    uint32_t width;
    uint32_t height;
} Point;

typedef struct Pixel {
    Point point;
    color_t color;
} Pixel;

/// @brief Generate random 32bit color
/// @return color 
color_t random_color() {
    color_t x = rand() & 0xff;
    x |= (rand() & 0xff) << 8;
    x |= (rand() & 0xff) << 16;
    x |= (rand() & 0xff) << 24;
    return x;
}

/// @brief Generate random hex colors
/// @param arr 
/// @param size 
void generate_random_colors(color_t* arr, size_t size) {
    for (size_t i = 0; i < size; i++)
    {
        arr[i] = random_color();
    }
}

void fill_image(Image img, color_t color) {
    for (size_t i = 0; i < HEIGHT; i++) {
        for (size_t j = 0; j < WIDTH; j++) {
            img[i][j] = color;
        }
    }
}

/// @brief Save image as .ppm file.
/// @param img Image to save to file.
/// @param filename path and filename. Must end with .ppm.
void save_image(Image img, const char* filename) {
    FILE* fp = fopen(filename, "w+");
    if (fp==NULL) {
        printf("File could not be created");
        exit(1);
    }
    fprintf(fp, "P6\n%d %d 255\n", WIDTH, HEIGHT);

    for (size_t i = 0; i < HEIGHT; i++) {
        for (size_t j = 0; j < WIDTH; j++) {
            uint32_t pixel = img[i][j];
            uint8_t bytes[3] = {
                (pixel&0x0000ff)>>8*0,
                (pixel&0x00ff00)>>8*1,
                (pixel&0xff0000)>>8*2,
            };
            fwrite(bytes, sizeof(bytes), 1, fp);
        }
    }
    fclose(fp);
}

void draw_square(Image img, uint32_t p_height, uint32_t p_width, color_t color, uint32_t square_width) {
    assert(p_width <= WIDTH && p_height <= HEIGHT);
    unsigned int radius = square_width / 2;
    for (size_t i = (p_height-radius) % HEIGHT; i <= (p_height+radius) % HEIGHT; i++) {
        for (size_t j = (p_width-radius) % WIDTH; j <= (p_width+radius) % WIDTH; j++) {
            img[i][j] = color;
        }
    }
}

void generate_random_points(Point* arr, size_t size) {
    for (size_t i = 0; i < size; i++) {
        int ran_h = rand() % HEIGHT;
        int ran_w = rand() % WIDTH;
        Point p = { .height = ran_h, .width = ran_w };
        arr[i] = p; 
    }
}

void generate_random_pixels(Pixel* arr, size_t size) {
    Point* ran_points = malloc(sizeof(Point)*size);
    color_t* ran_colors = malloc(sizeof(color_t)*size);
    generate_random_points(ran_points, size);
    generate_random_colors(ran_colors, size);
    for (size_t i = 0; i < size; i++) {
        Pixel p = { .point=ran_points[i], .color=ran_colors[i] };
        arr[i] = p;
    }
    free(ran_points);
    free(ran_colors);
}

void draw_ran_shapes(Image img, int num, int size, void (*shape_drawer)(Image, uint32_t, uint32_t, color_t, size_t)) {
    Point* points = malloc(sizeof(Point)*num);
    generate_random_points(points, num);
    for (size_t i = 0; i < num; i++) {
        shape_drawer(img, points[i].height, points[i].width, COLOR_BLACK, size);
    }
    free(points);
}

void draw_pixels(
    Image img, 
    Pixel* arr, 
    size_t arr_size, 
    int pixel_size, 
    void (*shape_drawer)(Image, uint32_t, uint32_t, color_t, size_t)
) {
    for (size_t i = 0; i < arr_size; i++) {
        shape_drawer(
            img, arr[i].point.height, 
            arr[i].point.width, 
            arr[i].color, 
            pixel_size
        );
    }   
}

double euclidian_distance(Point p, Point q) {
    double p1 = (double)p.height;
    double p2 = (double)p.width;
    double q1 = (double)q.height;
    double q2 = (double)q.width;
    return sqrt(pow(q1 - p1, 2) + pow(q2 - p2, 2));
}

Pixel find_closest_pixel(Pixel* arr, size_t size, Point pixel_point) {
    Pixel closest_pixel = arr[0];
    uint32_t prev_min_dist = euclidian_distance(arr[0].point, pixel_point);
    for (size_t i = 0; i < size; i++) {
        uint32_t new_dist = euclidian_distance(arr[i].point, pixel_point);
        if (new_dist < prev_min_dist) {
            closest_pixel = arr[i];
            prev_min_dist = new_dist;
        }
    }
    return closest_pixel;
}

/// @brief Cluster each pixel in the image to a centroid, and color 
///        it the same as the centroid pixel.
/// @param img image to cluster pixels on
/// @param arr centroids
/// @param size number of centroids
void cluster_pixels(Image img, Pixel arr[], int size) {
    for (uint32_t i = 0; i < HEIGHT; i++) {
        for (uint32_t j = 0; j < WIDTH; j++) {
            Point pixel_point = { .height=i, .width=j };
            Pixel p = find_closest_pixel(arr, size, pixel_point);
            img[i][j] = p.color;
        }
    }
}

void cluster_procedural_image_generation(Image img) {
    fill_image(img, random_color());
    int ran_pixel_count = CENTROID_NUM;//(rand() % 40) + 5;
    Pixel* pixel_arr = (Pixel*)malloc(sizeof(Pixel)*ran_pixel_count);
    generate_random_pixels(pixel_arr, ran_pixel_count);
    cluster_pixels(img, pixel_arr, ran_pixel_count);
    // for (size_t i = 0; i < ran_pixel_count; i++)
    // {
    //     pixel_arr[i].color = COLOR_BLACK;
    // }
    // draw_pixels(img, pixel_arr, ran_pixel_count, 15, draw_square);
    free(pixel_arr);
}

int main() {

    int seed = 1001;
    srand(time(NULL));

    Image img;
    cluster_procedural_image_generation(img);
    save_image(img, "output.ppm");

    return 0;
}