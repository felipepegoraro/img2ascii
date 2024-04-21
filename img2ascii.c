#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define STB_DS_IMPLEMENTATION
#include "./inc/stb_ds.h"

#define STB_IMAGE_IMPLEMENTATION
#include "./inc/stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "./inc/stb_image_resize2.h"

const char *exec_name = "img2ascii";

typedef struct color {
  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a;
} Color;

typedef struct charmap_t { 
  int key;
  unsigned char value;
} charmap_t;

static charmap_t *map = NULL;

const int f = 20;

void init_map()
{
  const char str[] = "@#$%&XOo+=-*.";
  int idx = 0;
  for (int i=0; i<14; i++){
    hmput(map, idx, str[i]);
    idx += f;
  }
}

unsigned char get_char_range_based(int pos){
  return hmget(map, round(pos/f)*f);
}

unsigned char *load_image(const char *img_name, int *width, int *height, int *comp){
  unsigned char *image_data =  stbi_load(img_name, width, height, comp, 0);
  if (image_data == NULL)
    fprintf(stderr, "Erro ao carregar a imagem %s\n", img_name);
  return image_data;
}

Color *get_colors_from_image(unsigned char *image, int width, int height, int comp) {
  if (!image) {
    fprintf(stderr, "Erro: imagem não carregada.\n");
    return NULL;
  }

  Color *colors = malloc(sizeof(Color) * width * height);
  if (!colors) {
    fprintf(stderr, "Erro ao alocar memória para as cores da imagem.\n");
    return NULL;
  }

  int index = 0;
  for (int h = 0; h < height; h++) {
    for (int w = 0; w < width; w++) {
      int idx = h * width + w;
      colors[index].r = image[idx * comp];
      colors[index].g = image[idx * comp + 1];
      colors[index].b = image[idx * comp + 2];
      colors[index].a = (comp == 4) 
        ? image[(h * width + w) * comp + 3]
        : 255;
      index++;
    }
  }

  return colors;
}

unsigned char get_luminance(Color pixel)
{
  return (unsigned char)(
    pixel.r *  0.3f +
    pixel.g * 0.59f + 
    pixel.b * 0.11f 
  );
}

unsigned char *resize_image(
  unsigned char *image_data, int width, int height, int comp,
  int desired_width, int desired_height
){
  unsigned char *resized_image = malloc(desired_width * desired_height * comp);
  if (!resized_image) {
    fprintf(stderr, "Erro ao alocar memória para a imagem redimensionada.\n");
    return NULL;
  }

  stbir_resize_uint8_srgb(
    image_data, width, height, 0,
    resized_image, desired_width, desired_height, 0, comp
  );

  return resized_image;
}

int main(int argc, char *argv[])
{
  if (argc <= 2){
    printf("uso: %s [scale] [filename]\n", exec_name);
    return EXIT_FAILURE;
  }

  const char *image_name = argv[2];

  int width, height, comp;
  unsigned char *image_data = load_image(image_name, &width, &height, &comp);
  if (image_data == NULL) return EXIT_FAILURE;

  int desired_width = atoi(argv[1]);
  int desired_height = (int)((float)height * ((float)desired_width / width) * 0.5f);
  unsigned char mat[width][height];

  unsigned char *resized_image = resize_image(image_data, width, height, comp, desired_width, desired_height);
  if (resized_image == NULL) return EXIT_FAILURE;

  Color *colors = get_colors_from_image(resized_image, desired_width, desired_height, comp);
  init_map();

  if (colors != NULL){
    for (int h=0; h<desired_height; h++){
      for (int w=0; w<desired_width; w++){
        int index = h*desired_width+w;
        Color curr = colors[index];
        unsigned char lum = get_luminance(curr);
        mat[w][h] = get_char_range_based(lum);
      }
    }
  }

  for (int h=0; h<desired_height; h++){
    for (int w=0; w<desired_width; w++)
      printf("%c", mat[w][h]);
    printf("\n");
  }

  if (colors != NULL) {
    free(colors); 
    colors = NULL;
  }

  if (image_data != NULL) {
    stbi_image_free(image_data);
    image_data = NULL;
  }

  if (resized_image != NULL) {
    stbi_image_free(resized_image);
    resized_image = NULL;
  }

  if (map != NULL) {
    hmfree(map);
    map = NULL;
  }

  return EXIT_SUCCESS;
}
