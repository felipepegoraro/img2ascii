#define STB_IMAGE_IMPLEMENTATION
#include "./inc/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "./inc/stb_image_write.h"

#include <stdio.h>

enum fileType {
  GIF=0,
  PNG=1,
  ERR=2,
};

enum fileType test_file_type(FILE *file)
{
  if (file == NULL) return ERR;

  unsigned char bytes[8];
  fread(bytes, 1, 8, file);

  // gif signature: 47 49 46 38 39 61
  if (
    bytes[0] == 0x47 && bytes[1] == 0x49 && bytes[2] == 0x46 &&
    bytes[3] == 0x38 && bytes[4] == 0x39 && bytes[5] == 0x61
  ) return GIF;

  // png signature: 89 50 4e 47 0d 0a 1a 0a
  if (
    bytes[0] == 0x89 && bytes[1] == 0x50 && bytes[2] == 0x4e &&
    bytes[3] == 0x47 && bytes[4] == 0x0d && bytes[5] == 0x0a &&
    bytes[6] == 0x1a && bytes[7] == 0x0a
  ) return PNG;

  return ERR;
}

int main(int argc, char *argv[])
{
  if (argc < 2) {
    printf("main2 [file]\n");
    return -1;
  }

  FILE *f = fopen(argv[1], "rb");
  if (f == NULL) return -1;
  enum fileType type = test_file_type(f);
  fclose(f);

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

  unsigned char *buffer = (unsigned char*)malloc(len);
  if (buffer == NULL){
    fprintf(stderr, "Erro ao alocar memória\n");
    fclose(f);
    return -1;
  }

  size_t bytes = fread(buffer, 1, len, f);
  fclose(f);

  if (bytes != len){
    fprintf(stderr, "Erro ao ler o arquivo GIF\n");
    free(buffer);
  }

  int width, height, num_frames, channels;
  int *delays;

  stbi_uc *data = stbi_load_gif_from_memory(
    buffer,
    len,
    &delays,
    &width,
    &height,
    &num_frames,
    &channels,
    0
  );

  if (data == NULL) {
    printf("Erro ao carregar o arquivo GIF.\n");
    free(buffer);
    return 1;
  }

  printf("Dimensões: %d x %d\n", width, height);
  printf("Número de frames: %d\n", num_frames);
  printf("Canais de cor: %d\n", channels);

  printf("Informações do primeiro frame:\n");
  printf("Largura: %d\n", width);
  printf("Altura: %d\n", height);
  printf("Canais de cor: %d\n", channels);

  int frame_index = 50;
  size_t offset = width * height * channels * frame_index;
  stbi_uc *frame_data = data + offset;
  stbi_write_png("x.png", width, height, channels, frame_data, 0);

  stbi_image_free(data);
  free(buffer);

  return 0;
}
