#ifndef _PYTHON_INTEPRETER_H_
#define _PYTHON_INTEPRETER_H_

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif

#include <Python.h>

typedef enum PIResult {
    PI_OK = 0,
    PI_PYTHONPATH_ERR = -1,
    PI_PREINITIALIZE_ERR = -2,
    PI_INITIALIZE_ERR = -3,
    PI_COMPILE_ERR = -4,
    PI_EVALCODE_ERR = -5,
    PI_FUNC_NOT_FOUND_ERR = -6,
    PI_NOT_READY_ERR = -7,
    PI_UNKNOWN_ERR = -1000,
} PIResult;

// struct PythonInterpreter;
typedef struct PythonInterpreter {
    PyObject *compiled;
    PyObject *globals;
    PyObject *locals;
    PyObject *func;
    PyObject *func_add;
} PythonInterpreter;

// Initialize PythonInterpreter
PIResult py_interpreter_initialize(PythonInterpreter *interpreter, const char *func_name, int argc, char *argv[]);

// Free PythonInterpreter
void py_interpreter_free(PythonInterpreter *interpreter);

// Call 'hello_python' method
PIResult py_interpreter_hello_python(PythonInterpreter *interpreter);

// Call 'add' method
PIResult py_interpreter_add(PythonInterpreter *interpreter, unsigned long a, unsigned long b, unsigned long *result);

#endif
