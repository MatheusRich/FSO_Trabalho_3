#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


typedef struct buffer
{
  int buffer[50];
  int amount;
  int bigger_generated;
  int smaller_generated;
} Buffer;

int random_number()
{
  long random = rand()%RAND_MAX - rand()%RAND_MAX;

  return random;
}

void sleep_ms(int ms)
{
  usleep(ms*1000);
}

void initialize_buffer(Buffer *buffer)
{
  buffer->amount = 0;
  buffer->bigger_generated = 0;
  buffer->smaller_generated = 0;

  int i;
  for(i=0; i<50; i++)
  {
    buffer->buffer[i] = 0;
  }
}

void write_to_buffer(Buffer *buffer, int number)
{
  for(unsigned int i=0; i<50; i++)
  {
    if(buffer->buffer[i] == 0)
    {
      buffer->buffer[i] = number;
      buffer->amount++;

      break;
    }
  }
}

long read_from_buffer(Buffer *buffer)
{
  for(unsigned int i=0; i<50; i++)
  {
    if(buffer->buffer[i] != 0)
    {
      long aux = buffer->buffer[i];
      buffer->buffer[i] = 0; // Excluding from buffer
      return aux;
    }
  }
  // #TODO Caso não haja nada a ler, o que fazer??
  return -1;
}

// Thread produtora
void producerThread(Buffer *buffer)
{
  long number = random_number();

  write_to_buffer(buffer, number);
  sleep_ms(100);

  // Escrever no log
  printf("[producao]: Numero gerado: %ld\n", number);
}


// thread consumidora 1
void consumerThread(Buffer *buffer, char thread_id)
{
  long number = read_from_buffer(buffer);

  // Calcular maior
  // bigger_number_calculation(buffer, number);
  // Calcular menor
  // Escrever no log
  printf("[consumo %c]: Numero lido: %ld\n", thread_id, number);
  sleep_ms(150);

}
// thread consumidora 2

int main(int argc, char **argv)
{
  srand((unsigned)time(NULL));
  Buffer buffer;

  initialize_buffer(&buffer);

  char *output_string = argv[1];
  FILE *output_file = fopen(output_string, "a");

  if (output_file == NULL)
  {
    printf("ERROR: File could not be opened!\n");
    exit(-1);
  }

  for(unsigned int i=0;i<2;i++)
    producerThread(&buffer);
  consumerThread(&buffer, 'a');
  consumerThread(&buffer, 'b');

  return 0;
}
