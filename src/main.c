#define PY_SSIZE_T_CLEAN
#define _GNU_SOURCE
#include <libgen.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <Python.h>
#include <wchar.h>

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

int main(int argc, char *argv[]) {
    PyObject *pName, *pModule, *pFunc, *pArgs, *pValue;
    PyStatus status;
    PyPreConfig preconfig;
    PyConfig config;
    const char *filename = "src.hello";
    const char *func_name = "hello_python";
    // char *actualpath[PATH_MAX + 1];
    // char *ptr;
    // ptr = realpath(argv[0], actualpath);
    // ptr = dirname(actualpath);
    // ptr = dirname(ptr);
    // printf("command: '%s'\n", ptr);

    /* Get base source directory path */
    wchar_t *wcwd = get_pythonpath(NULL);
    if (wcwd == NULL) {
        printf("failed to read current directory.\n");
        return 1;
    }
    wprintf(L"current working dir: %ls\n", wcwd);

    /* Pre-initialize Python */
    PyPreConfig_InitPythonConfig(&preconfig);
    preconfig.utf8_mode = 1;
    preconfig.allocator = PYMEM_ALLOCATOR_MALLOC;
    status = Py_PreInitialize(&preconfig);
    if (PyStatus_Exception(status)) {
        Py_ExitStatusException(status);
        return 1;
    }

    /* Initialize Python Interpreter */
    PyConfig_InitPythonConfig(&config);
    config.pythonpath_env = wcwd;
    status = PyConfig_SetBytesArgv(&config, argc, argv);
    if (PyStatus_Exception(status)) {
        PyConfig_Clear(&config);
        Py_ExitStatusException(status);
        return 1;
    }

    /* optional but recommended */
    status = Py_InitializeFromConfig(&config);
    // Py_Initialize();
    if (PyStatus_Exception(status)) {
        PyConfig_Clear(&config);
        Py_ExitStatusException(status);
        return 1;
    }
    PyConfig_Clear(&config);

    pName = PyUnicode_DecodeFSDefault(filename);
    if (pName == NULL) {
        printf("deu ruim 4.\n");
        PyErr_Print();
        fprintf(stderr, "Failed to convert string to python object \"%s\"\n", filename);
        return 1;
    }

    pModule = PyImport_Import(pName);
    Py_DECREF(pName);
    if (pModule == NULL) {
        PyErr_Print();
        fprintf(stderr, "Failed to load \"%s\"\n", filename);
        return 1;
    }

    pFunc = PyObject_GetAttrString(pModule, func_name);
    if (pFunc == NULL || !PyCallable_Check(pFunc)) {
        if (PyErr_Occurred()) PyErr_Print();
        fprintf(stderr, "Cannot find function \"%s\"\n", func_name);
        return 1;
    }

    pArgs = PyTuple_New(0);
    if (pArgs == NULL) {
        if (PyErr_Occurred()) PyErr_Print();
        fprintf(stderr, "Failed to create tuple\n");
        return 1;
    }

    pValue = PyObject_CallObject(pFunc, pArgs);
    Py_DECREF(pArgs);

    if (pValue == NULL || pValue == Py_None) {
        fprintf(stdout, "returned null\n");
    }

    if (Py_FinalizeEx() < 0) {
        return 120;
    }
    return 0;
    // printf() displays the string inside quotation
    // printf("Hello, World!\n");

    //     PyStatus status;
    //     PyConfig config;
    //     PyConfig_InitPythonConfig(&config);

    //     /* optional but recommended */
    //     status = PyConfig_SetBytesString(&config, &config.program_name, argv[0]);
    //     if (PyStatus_Exception(status)) {
    //         goto exception;
    //     }

    //     status = Py_InitializeFromConfig(&config);
    //     if (PyStatus_Exception(status)) {
    //         goto exception;
    //     }
    //     PyConfig_Clear(&config);
    //     PyRun_SimpleString("def hello_python():\n"
    //                        "    print(\"hello from python\")\n");

    //     if (Py_FinalizeEx() < 0) {
    //         exit(120);
    //     }

    //     return 0;
    // exception:
    //     PyConfig_Clear(&config);
    //     Py_ExitStatusException(status);
    //     return 1;
}
