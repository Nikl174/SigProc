#include <assert.h>
#include <stdio.h>

#define STACK_CAPACITY 1024

/*! \enum
 *
 *  Instruction types implemented in the stack-machine
 */
#include <stddef.h>
typedef enum {
  INST_PUSH, /*! Push element onto the stack */
  INST_POP,  /*! Remove the top most element from stack */
  INST_ADD,  /*! Add both top most elements of stack and write on top */
  INST_SUB, /*! Subtract second- from first-top-most elements of stack and write
               on top */
  INST_PRINT /*! Print out top most element */
} InstType;

/*! \struct
 *  \brief Instruction
 *
 *  General Structure of an instruction
 */
typedef struct {
  InstType type; /*!< General Structure of an instruction */
  int operant;   /*!< Operant to use instruction on*/
} Inst;

Inst programm[] = {{.type = INST_PUSH, .operant = 20},
                   {.type = INST_PUSH, .operant = 22},
                   {.type = INST_ADD},
                   {.type = INST_PRINT},
                   {.type = INST_PUSH, .operant = 40},
                   {.type = INST_SUB},
                   {.type = INST_PRINT},
                   {.type = INST_POP},
                   {.type = INST_PRINT}};

// use the size of the allocated list (itemsize * items_in_list) and divide with the size of one element
#define NUM_OF_INSTR (sizeof(programm) / sizeof(programm[0]))

/*! \struct Stack
 *  \brief Stack data-structure to work on
 *
 *  Has the actual stack and a pointer pointing to the next free element
 */
typedef struct {
  int st_data[STACK_CAPACITY]; /*!< The stack data */
  size_t st_pointer;           /*!< Current top free element of the stack */
} Stack;

/*! \brief Implementation of the stack-push operation
 *
 * \param [out] stack stack top work on
 * \param [in] value integer to put on top of the stack
 */
void stack_push(Stack *stack, int value) {
  assert(stack->st_pointer < STACK_CAPACITY - 1);
  stack->st_data[stack->st_pointer++] = value;
}

/*! \brief Implementation of the stack-pop operation
 *
 * Removes the element by JUST decreasing the pointer, so not actually erased!
 *
 * \param [out] stack stack top work on
 *
 * \return the top most element
 */
int stack_pop(Stack *stack) {
  assert(stack->st_pointer > 0);
  return stack->st_data[--stack->st_pointer];
}

/*! \brief Implementation of the stack-top operation
 *
 * \param [out] stack stack top work on
 *
 * \return the top most element
 */
int stack_top(Stack *stack) {
  assert(stack->st_pointer >= 0);
  return stack->st_data[stack->st_pointer - 1];
}

/*! \brief Saves the programm to a binary file
 */
void save_programm_to_file(const char *filename) {
  FILE* file = fopen(filename, "wb");
  fwrite(programm, sizeof(programm[0]), NUM_OF_INSTR, file);
  fclose(file);
}

int main(int argc, char *argv[]) {
  Stack stack;

  save_programm_to_file("programm.bin");

  // implements the stack operations and executes the programm
  for (size_t ip = 0; ip < NUM_OF_INSTR; ++ip) {
    switch (programm[ip].type) {
    case INST_PUSH:
      stack_push(&stack, programm[ip].operant);
      break;
    case INST_POP:
      stack_pop(&stack);
      break;
    case INST_ADD: {
      int a = stack_pop(&stack);
      int b = stack_pop(&stack);
      stack_push(&stack, a + b);
    } break;
    case INST_SUB: {
      int a = stack_pop(&stack);
      int b = stack_pop(&stack);
      stack_push(&stack, b - a);
    } break;
    case INST_PRINT:
      printf("%d\n", stack_top(&stack));
      break;
    default:
      assert(0 && "Invalid instruction");
    }
  }
}
