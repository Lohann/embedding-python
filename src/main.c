#include <stdio.h>
#include <stdlib.h>
#include "python_interpreter.h"

// Run python's `add` method 100 times.
int run_add(PythonInterpreter *interpreter) {
    int i;
    unsigned long a, b, result;

    for (i = 0; i < 100; i++)
    {
        a = (unsigned long) rand();
        b = (unsigned long) rand();
        if (py_interpreter_add(interpreter, a, b, &result) != PI_OK)
        {
            printf("add failed\n");
            return 1;
        }
        printf("%li + %li = %li\n", a, b, result);
        if (a + b != result) {
            printf("wrong result, expect %li, got: %li.\n", a+b, result);
            return 1;
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    PythonInterpreter interpreter;
    const char *func_name = "hello_python";

    // Initialize the randomizer using the current timestamp as a seed
    // (The time() function is provided by the <time.h> header file)
    srand(time(NULL));

    // Initialize interpreter
    if (py_interpreter_initialize(&interpreter, func_name, argc, argv) != PI_OK) {
        printf("load interpreter failed.\n");
        return 1;
    }

    // Call `hello_python` function
    py_interpreter_hello_python(&interpreter);
    py_interpreter_hello_python(&interpreter);

    // Call `add` function
    unsigned long result;
    if (py_interpreter_add(&interpreter, 5, 7, &result) != PI_OK) {
        printf("add failed\n");
        return 1;
    }
    printf("5 + 7 = %li\n", result);

    // Call `add` 100 times with random values.
    run_add(&interpreter);

    // Cleanup interpreter memory
    py_interpreter_free(&interpreter);

    return 0;
}
