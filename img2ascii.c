#define STB_IMAGE_IMPLEMENTATION
#include "./inc/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "./inc/stb_image_write.h"

#define STB_DS_IMPLEMENTATION
#include "./inc/stb_ds.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "./inc/stb_image_resize2.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct charmap_t {
  int key;
  stbi_uc value;
} charmap_t;

static charmap_t *map = NULL;

typedef struct color {
  stbi_uc r;
  stbi_uc g;
  stbi_uc b;
  stbi_uc a;
} Color;

const int f = 20;

void init_map() {
  const char str[] = "@#$%&XOo+=-*.";
  int idx = 0;
  for (int i = 0; i < 14; i++) {
    hmput(map, idx, str[i]);
    idx += f;
  }
}

stbi_uc get_char_range_based(int pos) {
  return hmget(map, round(pos / f) * f);
}

stbi_uc *resize_image(
  stbi_uc *image_data, int width, int height, int comp,
  int desired_width, int desired_height
){
  stbi_uc *resized_image = malloc(desired_width * desired_height * comp);
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

Color *get_colors_from_image(stbi_uc *image, int width, int height, int comp) {
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

stbi_uc get_luminance(Color pixel) {
  return (stbi_uc) (
    pixel.r * 0.3f +
    pixel.g * 0.59f +
    pixel.b * 0.11f
  );
}

enum fileType {
  GIF = 0,
  PNG = 1,
  ERR = 2,
};

enum fileType test_file_type(FILE *file) {
  if (file == NULL) return ERR;

  stbi_uc bytes[8];
  fread(bytes, 1, 8, file);

  // 47 49 46 38 39 61
  if (
    bytes[0] == 0x47 && bytes[1] == 0x49 && bytes[2] == 0x46 &&
    bytes[3] == 0x38 && bytes[4] == 0x39 && bytes[5] == 0x61
  ) return GIF;

  // 89 50 4e 47 0d 0a 1a 0a
  if (
    bytes[0] == 0x89 && bytes[1] == 0x50 && bytes[2] == 0x4e &&
    bytes[3] == 0x47 && bytes[4] == 0x0d && bytes[5] == 0x0a &&
    bytes[6] == 0x1a && bytes[7] == 0x0a
  ) return PNG;

  return ERR;
}

int main(int argc, char *argv[]) {
  if (argc <= 2) {
    printf("main2 [size] [file]\n");
    return -1;
  }

  FILE *f = fopen(argv[2], "rb");
  if (f == NULL) return -1;
  enum fileType type = test_file_type(f);

  if (type == GIF) {
    printf("File is a GIF.\n");
  } else if (type == PNG) {
    printf("File is a PNG.\n");
  } else {
    printf("File type is not recognized.\n");
  }

  long len = 0;
  fseek(f, 0, SEEK_END);
  len = ftell(f);
  fseek(f, 0, SEEK_SET);

  stbi_uc *buffer = (stbi_uc *) malloc(len);
  if (buffer == NULL) {
    fprintf(stderr, "Erro ao alocar memória\n");
    fclose(f);
    return -1;
  }

  size_t bytes = fread(buffer, 1, len, f);
  fclose(f);

  if (bytes != len) {
    fprintf(stderr, "Erro ao ler o arquivo GIF\n");
    free(buffer);
  }

  int width, height, num_frames, channels;
  int *delays = NULL;

  stbi_uc *data = stbi_load_gif_from_memory( buffer, len, &delays, &width, &height, &num_frames, &channels, 0);
  if (data != NULL) free(delays);
  else {
    printf("Erro ao carregar o arquivo GIF.\n");
    free(buffer);
    return 1;
  }

  printf("Dimensões: %d x %d\n", width, height);
  printf("Número de frames: %d\n", num_frames);
  printf("Canais de cor: %d\n", channels);

  int desired_width = atoi(argv[1]);
  int desired_height = (int) ((float) height * ((float) desired_width / width) * 0.5f);

  init_map();
  stbi_uc ***mat = malloc(num_frames * sizeof(stbi_uc **));

  for (int frame = 0; frame < num_frames; frame++) {
    size_t offset = width * height * channels * frame;
    stbi_uc *frame_data = data + offset;

    stbi_uc *resized_image = resize_image(frame_data, width, height, channels, desired_width, desired_height);
    if (resized_image == NULL) {
      // TODO: free.
      return EXIT_FAILURE;
    }

    Color *colors = get_colors_from_image(resized_image, desired_width, desired_height, channels);
    if (colors != NULL) {
      mat[frame] = malloc(desired_width * sizeof(stbi_uc *));
      for (int h = 0; h < desired_height; h++) {
        mat[frame][h] = malloc(desired_width * sizeof(stbi_uc));
        for (int w = 0; w < desired_width; w++) {
          int index = h * desired_width + w;
          Color curr = colors[index];
          stbi_uc lum = get_luminance(curr);
          mat[frame][h][w] = get_char_range_based(lum);
        }
      }
      free(colors);
    }

    free(resized_image);
  }

  stbi_image_free(data);
  data = NULL;

  for (int frame = 0; frame < num_frames; frame++){
    printf("Frame %d =====================================================\n", frame + 1);
    for (int h = 0; h < desired_height; h++) {
      for (int w = 0; w < desired_width; w++)
        printf("%c", mat[frame][h][w]);
      free(mat[frame][h]);
      printf("\n");
    }
    free(mat[frame]);
  }
  free(mat);

  free(buffer);
  hmfree(map);

  return 0;
}

