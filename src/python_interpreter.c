#define PY_SSIZE_T_CLEAN
#define _GNU_SOURCE
#include <libgen.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <Python.h>
#include <wchar.h>
#include <stdbool.h>
#include "config.h"
#include "python_interpreter.h"

// Get python's base source directory
wchar_t* get_pythonpath(wchar_t *output) {
    char cwd[PATH_MAX];
    size_t strSize;
    if (getcwd(cwd, sizeof(cwd)) == NULL) return NULL;
    strSize = strlen(cwd)+1;
    if (output == NULL) {
        output = malloc(sizeof(wchar_t) * strSize);
    }
    mbstowcs(output, (char *)&cwd, strSize);
    return output;
}

// Get a method from dictionary
PyObject* get_func_ref(PyObject* dict, const char *func_name) {
    PyObject *key;
    PyObject *func;

    // Fail if 'dict' is not a python's dictionary or is empty.
    if (PyDict_Check(dict) == 0 || PyDict_Size(dict) == 0) {
        return NULL;
    }

    // Convert C string to Python's string
    key = PyUnicode_DecodeFSDefault(func_name);
    if (key == NULL) {
        PyErr_Print();
        return NULL;
    }

    // Return the object from dictionary *op* which has a key *key*.
    // - If the key is present, set *result to a new strong reference to the value
    //   and return 1.
    // - If the key is missing, set *result to NULL and return 0 .
    // - On error, raise an exception and return -1.
    if (PyDict_GetItemRef(dict, key, &func) != 1 || !PyCallable_Check(func)) {
        func = NULL;
    }

    // Release string
    Py_DECREF(key);
    return func;
}

void py_interpreter_free(PythonInterpreter *interpreter) {
    if (interpreter == NULL) return;
    // Undo all initializations made by `Py_InitializeFromConfig` and subsequent
    // use of Python/C API functions, and destroy all sub-interpreters.
    Py_FinalizeEx();
    interpreter->globals = NULL;
    interpreter->locals = NULL;
    interpreter->func = NULL;
    interpreter->compiled = NULL;
}

PIResult py_interpreter_initialize(PythonInterpreter *interpreter, const char* func_name, int argc, char *argv[])
{
    PyStatus status;
    PyPreConfig preconfig;
    PyConfig config;
    wchar_t *python_path;

    // Set all values to zero
    memset(interpreter, 0, sizeof(PythonInterpreter));

    /* Get base source directory path */
    python_path = get_pythonpath(NULL);

    if (python_path == NULL) {
        return PI_PYTHONPATH_ERR;
    }

    //////////////////////////////////////
    // Configure Pre-initialize options //
    //////////////////////////////////////
    PyPreConfig_InitPythonConfig(&preconfig);
    // Enable PEP-0540
    preconfig.utf8_mode = 1;
    // Use malloc as memory allocator
    preconfig.allocator = PYMEM_ALLOCATOR_MALLOC;
    // Set pre-initialize options
    status = Py_PreInitialize(&preconfig);
    if (PyStatus_Exception(status)) {
        Py_ExitStatusException(status);
        return PI_PREINITIALIZE_ERR;
    }

    ////////////////////////////////////
    // Configure Python's Interpreter //
    ////////////////////////////////////
    // 1 - Initialize config object with default values.
    PyConfig_InitPythonConfig(&config);

    // 2 - Set source base_path.
    config.pythonpath_env = python_path;

    // 3 - Set Python's Interpreter argc and argv
    status = PyConfig_SetBytesArgv(&config, argc, argv);
    if (PyStatus_Exception(status)) {
        PyConfig_Clear(&config);
        py_interpreter_free(interpreter);
        Py_ExitStatusException(status);
        return PI_PREINITIALIZE_ERR;
    }

    ///////////////////////////////////
    // Initialize Python Interpreter //
    ///////////////////////////////////
    // 1 - Initialize interpreter with configuration
    status = Py_InitializeFromConfig(&config);
    if (PyStatus_Exception(status)) {
        PyConfig_Clear(&config);
        Py_ExitStatusException(status);
        return PI_INITIALIZE_ERR;
    }

    // Release config struct from memory.
    PyConfig_Clear(&config);
    
    // 2 - Compile python code
    interpreter->compiled = Py_CompileString((const char*)python_source, "main.py", Py_file_input);
    if (interpreter->compiled == NULL) {
        if (PyErr_Occurred()) {
            PyErr_Print();
        }
        py_interpreter_free(interpreter);
        return PI_COMPILE_ERR;
    }

    // 3 - Run code
    interpreter->globals = PyDict_New();
    interpreter->locals = PyDict_New();
    PyObject *returned_value = PyEval_EvalCode(interpreter->compiled, interpreter->globals, interpreter->locals);
    if (returned_value != Py_None) {
        if (PyErr_Occurred()) PyErr_Print();
        py_interpreter_free(interpreter);
        return PI_EVALCODE_ERR;
    }

    // 4 - Extract python function
    interpreter->func = get_func_ref(interpreter->locals, func_name);
    if (interpreter->func == NULL) {
        if (PyErr_Occurred()) PyErr_Print();
        py_interpreter_free(interpreter);
        return PI_FUNC_NOT_FOUND_ERR;
    }

    // 5 - Extract python function
    interpreter->func_add = get_func_ref(interpreter->locals, "add");
    if (interpreter->func == NULL) {
        if (PyErr_Occurred()) PyErr_Print();
        py_interpreter_free(interpreter);
        return PI_FUNC_NOT_FOUND_ERR;
    }
    return PI_OK;
}

PIResult py_interpreter_hello_python(PythonInterpreter *interpreter) {
    PyObject *returnValue;

    // Check if interpreter is initialized.
    if (interpreter == NULL || interpreter->func == NULL) {
        return PI_NOT_READY_ERR;
    }

    // Call python function without arguments
    returnValue = PyObject_CallObject(interpreter->func, NULL);

    if (returnValue == NULL) {
        if (PyErr_Occurred()) PyErr_Print();
        return PI_UNKNOWN_ERR;
    }

    return PI_OK;
}

PIResult py_interpreter_add(PythonInterpreter *interpreter, unsigned long a, unsigned long b, unsigned long *result) {
    PyObject *pA, *pB, *args, *returnedValue;
    PyGILState_STATE gstate;
    PIResult status;

    // Check if interpreter is initialized.
    if (interpreter == NULL || interpreter->func_add == NULL) {
        return PI_NOT_READY_ERR;
    }

    // Initialize local variables.
    pA = NULL;
    pB = NULL;
    args = NULL;
    returnedValue = NULL;
    status = PI_UNKNOWN_ERR;

    // Acquire GIL (Global Interpreter Lock)
    gstate = PyGILState_Ensure();

    // Prepare method arguments by converting 'unsigned long' into `PyObject`.
    pA = PyLong_FromUnsignedLong(a);
    pB = PyLong_FromUnsignedLong(b);
    args = PyTuple_New(2);
    if (pA == NULL || pB == NULL || args == NULL)
        goto end;

    // Set tuple values
    if (PyTuple_SetItem(args, 0, pA) != 0) goto end;
    if (PyTuple_SetItem(args, 1, pB) != 0) goto end;

    // Call python function.
    returnedValue = PyObject_CallObject(interpreter->func_add, args);
    if (returnedValue == NULL || PyLong_Check(returnedValue) == 0)
        goto end;

    // Convert PyLong back to unsigned long
    *result = PyLong_AsUnsignedLong(returnedValue);
    if (PyErr_Occurred()) goto end;

    // Set status ok
    status = PI_OK;
end:
    // Release GIL
    PyGILState_Release(gstate);

    if (PyErr_Occurred()) PyErr_Print();
    // Cleanup memory
    if (returnedValue != NULL && Py_REFCNT(returnedValue) > 0) Py_DECREF(returnedValue);
    if (args != NULL && Py_REFCNT(args) > 0) Py_DECREF(args);
    return status;
}
