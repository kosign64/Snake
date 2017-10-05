#include <avr/io.h>
#include <stdlib.h>
#include <stdbool.h>
#include <util/delay.h>

typedef unsigned char uchar;
typedef signed char schar;

typedef enum
{
    DEAD,
    UP,
    RIGHT,
    DOWN,
    LEFT
}Direction;

typedef struct _snakeNode
{
  uchar positionX;
  uchar positionY;
  struct _snakeNode *nextNode;
  struct _snakeNode *previousNode;
}snakeNode;

snakeNode *head;
snakeNode *tail;

bool snakeMove(Direction direction, uchar *applePosition);
void prepareArray(uchar *array, uchar applePosition[]);
uchar keyboardScan(Direction direction);
void drawPicture(uchar imageArray[]);
void restart(void);

int main(void)
{
  uchar dotDisplay[32] = {0};
  uchar scullArray[] = {0, 0, 0, 0, 0, 0, 192, 192, 220, 192,
                        34, 33, 69, 18, 113, 12, 113, 12, 69,
                        18, 34, 33, 220, 192, 192, 192, 0, 0,
                        0, 0, 0, 0};
  uchar applePosition[2];
  Direction snakeDirection = RIGHT;
  bool gameOver = false;

  // PORTS initialization
  PORTA = _BV(0) | _BV(1) | _BV(2) | _BV(3); // Pull-up keyboard cols
  DDRC = 0xFF; // Dot-Matrix
  DDRD = 0xF0; // for Dot-matrix control
  DDRD |= _BV(0) | _BV(1) | _BV(2); // for keyboard

  head = (snakeNode*)malloc(sizeof(snakeNode));
  tail = (snakeNode*)malloc(sizeof(snakeNode));
  applePosition[0] = rand() % 16 ;
  applePosition[1] = rand() % 16;
  head->positionX = 1;
  head->positionY = 0;
  head->previousNode = NULL;
  tail->positionX = 0;
  tail->positionY = 0;
  head->nextNode = tail;
  tail->nextNode = NULL;
  tail->previousNode = head;

  while(1)
  {
    if(!gameOver)
    {
      gameOver = snakeMove(snakeDirection, applePosition);
      if(gameOver)
      {
        snakeDirection = DEAD;
      }
      if(!gameOver)
      {
        unsigned int i;
        prepareArray(dotDisplay, applePosition);
        for(i = 0; i < 30; ++i)
        {
          snakeDirection = keyboardScan(snakeDirection);
          drawPicture(dotDisplay);
        }
      }
    }
    else
    {
      if(snakeDirection != DEAD)
      {
        gameOver = false;
        restart();
      }
      else
      {
        drawPicture(scullArray);
        snakeDirection = keyboardScan(snakeDirection);
      }
    }
  }
}

bool snakeMove(Direction direction, uchar applePosition[])
{
  bool over = false;
  snakeNode *newNode = (snakeNode*)malloc(sizeof(snakeNode));
  snakeNode *tempNode;
  newNode->positionX = head->positionX;
  newNode->positionY = head->positionY;
  newNode->previousNode = NULL;
  newNode->nextNode = head;
  head->previousNode = newNode;
  switch(direction)
  {
  case UP:
    newNode->positionY--;
    break;

  case RIGHT:
    newNode->positionX++;
    break;

  case DOWN:
    newNode->positionY++;
    break;

  case LEFT:
    newNode->positionX--;
    break;
  }
  head = newNode;
  // if apple is eaten
  if((head->positionX == applePosition[0]) &&
          (head->positionY == applePosition[1]))
  {
    // Create a new apple
    bool insideSnake = true;
    snakeNode *tempAppleNode = head;
    while(insideSnake)
    {
      insideSnake = false;
      applePosition[0] = rand() % 16;
      applePosition[1] = rand() % 16;
      while(tempAppleNode != NULL)
      {
        if((tempAppleNode->positionX == applePosition[0]) &&
                (tempAppleNode->positionY == applePosition[1]))
        {
          insideSnake = true;
        }
        tempAppleNode = tempAppleNode->nextNode;
      }
    }
  }
  else
  {
    newNode = tail;
    tail = tail->previousNode;
    tail->nextNode = NULL;
    free((snakeNode*)newNode);
  }
  if(head->positionX >= 16 || head->positionY >= 16)
  {
    over = true;
  }
  tempNode = head->nextNode;
  while(tempNode->nextNode != NULL)
  {
    if((head->positionX == tempNode->positionX) &&
            (head->positionY == tempNode->positionY))
    {
      over = true;
    }
    tempNode = tempNode->nextNode;
  }

  return over;
}

void prepareArray(uchar *array, uchar applePosition[])
{
  uchar bitDisplay[16][16] = {{0}};
  snakeNode *tempNode;
  uchar currentByte;
  tempNode = head;
  // Fill bit array
  while(tempNode != NULL)
  {
    bitDisplay[tempNode->positionX][tempNode->positionY] = 1;
    tempNode = tempNode->nextNode;
  }
  bitDisplay[applePosition[0]][applePosition[1]] = 1;
  // Build array for drawPicture function
  for(uchar i = 0; i < 16; ++i)
  {
    for(uchar j = 0; j < 16; j += 8)
    {
      currentByte = 0;
      for(uchar k = 0; k < 8; ++k)
      {
        if(bitDisplay[i][j + k])
        {
          currentByte |= _BV(k);
        }
      }
      *array = currentByte;
      ++array;
    }
  }
}

unsigned char keyboardScan(Direction direction)
{
  Direction dir = direction;

  // Redefine Port A for keyboard scan
  DDRA = 0x00;
  PORTA = 0x00;
  // Scan of 2, 4, 6, 8 buttons
  PORTD &= ~(_BV(0) | _BV(1) | _BV(2));
  PORTD |= _BV(0);
  _delay_us(10);
  if(bit_is_set(PINA, 1))
  {
    dir = RIGHT; // Button 6 is pressed
  }
  PORTD &= ~_BV(0);
  PORTD |= _BV(1);
  _delay_us(10);
  if(bit_is_set(PINA, 0))
  {
    dir = UP; // Button 2 is pressed
  }
  if(bit_is_set(PINA, 2))
  {
    dir = DOWN; // Button 8 is pressed
  }
  PORTD &= ~_BV(1);
  PORTD |= _BV(2);
  _delay_us(10);
  if(bit_is_set(PINA, 1))
  {
    dir = LEFT; // Button 4 is pressed
  }
  PORTD &= ~_BV(2);

  return dir;
}

void restart(void)
{
  snakeNode *tempNode = head->nextNode;
  while(tempNode->nextNode != NULL)
  {
    tempNode = tempNode->nextNode;
    free((snakeNode*)tempNode->previousNode);
  }
  tail = tempNode;
  head->nextNode = tail;
  tail->previousNode = head;
  tail->nextNode = NULL;
  head->positionX = 1;
  head->positionY = 0;
  tail->positionX = 0;
  tail->positionY = 0;
}

void drawPicture(uchar imageArray[])
{
  // Redefine Port A for Dot-matrix
  PORTA = 0x00;
  DDRA = 0xFF;
  for(uchar i = 0; i < 16; ++i)
  {
    PORTD &= ~(0xF0);
    PORTD |= (i << 4);
    if(i < 8)
    {
      PORTA = imageArray[2 * i];
      PORTC = imageArray[2 * i + 16];
    }
    else
    {
      PORTA = imageArray[2 * (i - 8) + 1];
      PORTC = imageArray[2 * (i - 8) + 17];
    }
    _delay_us(200);
  }
}
