#include <Python.h>
#include <stdarg.h>
#include "../include/jinja2.h"

/**
 * Creates a new environment for Jinja and returns a pointer to the PyObject
 * Before the program exits you should call free_environment to all python
 * and c to clean up.
 * This function takes a string for the absolute path to the template directory
 */

 // TODO refactor this to make clean up easier
void* init_environment(const char* template_dir) {
    PyObject *pString, *pmodEnvironment, *pmodFileSystemLoader, *pDict;
    PyObject *pClassEnv, *pClassFLoader, *pArgs, *pFileLoader, *ret_env;
    PyObject *empty_tuple;

    // from jinja2 import Environment, FileSystemLoader
    Py_Initialize();
    pString = PyString_FromString("jinja2.environment");
    pmodEnvironment = PyImport_Import(pString);
    Py_DECREF(pString);
    if (!pmodEnvironment) {
        fprintf(stderr, "Error finding module jinja2\n");
        PyErr_Print();
        Py_Finalize();
        return NULL;
    }
    pString = PyString_FromString("jinja2.loaders");
    pmodFileSystemLoader = PyImport_Import(pString);
    Py_DECREF(pString);

    if (!pmodEnvironment) {
        fprintf(stderr, "Error finding module jinja2\n");
        PyErr_Print();
        Py_Finalize();
        return NULL;
    }
    // env = Environment(loader=FileSystemLoader(template_dir))
    // Get a reference to the Environment class
    pDict = PyModule_GetDict(pmodEnvironment);
    pClassEnv = PyDict_GetItemString(pDict, "Environment");

    if (!pClassEnv || !PyCallable_Check(pClassEnv)){
        fprintf(stderr, "Could not import Environment\n");
        PyErr_Print();
        Py_DECREF(pmodEnvironment);
        Py_DECREF(pmodFileSystemLoader);
        Py_Finalize();
        return NULL;
    }

    // get a reference to the FileSystemLoader class
    pDict = PyModule_GetDict(pmodFileSystemLoader);
    pClassFLoader = PyDict_GetItemString(pDict, "FileSystemLoader");

    if (!pClassFLoader || ! PyCallable_Check(pClassFLoader)) {
        fprintf(stderr, "Could not import FileSystemLoader\n");
        PyErr_Print();
        Py_DECREF(pClassEnv);
        Py_DECREF(pmodEnvironment);
        Py_DECREF(pmodFileSystemLoader);
        Py_Finalize();
        return NULL;
    }

    // now make an instance of the FileSystemLoader to pass to the Environment
    pArgs = Py_BuildValue("(s)", template_dir);
    pFileLoader = PyObject_Call(pClassFLoader, pArgs, NULL);
    Py_DECREF(pArgs);
    if (pFileLoader == NULL) {
        fprintf(stderr, "Could not instantiate a new FileSystemLoader\n");
        PyErr_Print();
        Py_DECREF(pClassEnv);
        Py_DECREF(pClassFLoader);
        Py_DECREF(pmodEnvironment);
        Py_DECREF(pmodFileSystemLoader);
        return NULL;
    }

    // Create an instance of the Environment class
    empty_tuple = PyTuple_New(0);
    pArgs = Py_BuildValue("{sO}", "loader", pFileLoader);
    ret_env = PyObject_Call(pClassEnv, empty_tuple, pArgs);
    Py_DECREF(pArgs);
    Py_DECREF(empty_tuple);

    if (ret_env == NULL) {
        fprintf(stderr, "Could not instantiate a new Environment\n");
        PyErr_Print();
        Py_DECREF(pFileLoader);
        Py_DECREF(pClassEnv);
        Py_DECREF(pClassFLoader);
        Py_DECREF(pmodEnvironment);
        Py_DECREF(pmodFileSystemLoader);
        return NULL;
    }
    Py_DECREF(pFileLoader);
    Py_DECREF(pClassEnv);
    Py_DECREF(pClassFLoader);
    Py_DECREF(pmodEnvironment);
    Py_DECREF(pmodFileSystemLoader);
    return ret_env;
}

void free_environment(void* env) {
    PyObject* pEnv = (PyObject*) env;
    if (pEnv) {
        Py_DECREF(pEnv);
        Py_Finalize();
    }
}

void* get_template(void* env, const char* template_name) {
    PyObject *retval;
    PyObject* pEnv = (PyObject*) env;
    retval = PyObject_CallMethod(pEnv, "get_template", "(s)", template_name);
    if (!retval) {
        fprintf(stderr, "Could not load template %s\n", template_name);
        // PyErr_Print();
        return NULL;
    }
    return (void*)retval;
}

// TODO support optional extensions and filter_func
void list_templates(void* env) {
    PyObject *retval, *iterator, *item;
    PyObject* pEnv = (PyObject*) env;
    retval = PyObject_CallMethod(pEnv, "list_templates", NULL);
    if (!retval) {
        fprintf(stderr, "Could not list templates \n");
        PyErr_Print();
        return;
    }

    iterator = PyObject_GetIter(retval);
    if (iterator == NULL) {
        fprintf(stderr, "Could not list templates \n");
        PyErr_Print();
        Py_DECREF(retval);
        return;
    }

    while ((item = PyIter_Next(iterator))) {
        printf("%s\n", PyString_AsString(item));
        Py_DECREF(item);
    }

    Py_DECREF(iterator);

    if (PyErr_Occurred()) {
        PyErr_Print();
        Py_DECREF(retval);
    }
}



/**
 * Calls the render function on this template and returns the rendered string. 
 * Do not call free on the rendered string!
 *
 * int count, the number of string arguments provided to that function
 * args is an array of strings that contain both keys and values
 * ex: 
 * render(tmpl, 2, ["name", "James"]);
 * render(tmpl, 4, ["name", "James", "age", "24"]);
 */
char* render(void *tmpl, int count, char** args) {
    PyObject *retval, *pArgs, *pString;
    PyObject* pTmpl = (PyObject*) tmpl;
    int i;

    if (count % 2) {
        fprintf(stderr, "The number of items given must be  \n");
        return NULL;
    }

    if (count == 0) {
        pArgs = PyTuple_New(0);
        pString = PyString_FromString("render");
        retval = PyObject_CallMethodObjArgs(pTmpl, pString, pArgs, NULL);
        Py_DECREF(pString);
        Py_DECREF(pArgs);
    } else {
        pArgs = PyDict_New();
        for (i=0; i<count; i+=2) {
            pString = PyString_FromString(args[i+1]);
            PyDict_SetItemString(pArgs, args[i], pString);
            Py_DECREF(pString);
        }

        pString = PyString_FromString("render");
        retval = PyObject_CallMethodObjArgs(pTmpl, pString, pArgs, NULL);
        Py_DECREF(pString);
        Py_DECREF(pArgs);
    }
    if (!retval) {
        fprintf(stderr, "Could not list templates \n");
        PyErr_Print();
        return NULL;
    }
    return PyString_AsString(retval);
}

// int main(int argc, char *argv[])
// {
//     PyObject *env, *tmpl;
//     char *rendered;
//     char **vars;
//     // hardcoded to my path for now, if you want to test this you should change this directory
//     env = init_environment("/home/jrowe7/slf/jinja2-c/test_template/");
//     if (!env) {
//         fprintf(stderr, "Could not instantiate a new Environment\n");
//         return -1;
//     }

//     list_templates(env);

//     tmpl = get_template(env, "index.html");
//     // badval = get_template(env, "dne");
//     vars = malloc( sizeof(char*) * 2 );
//     vars[0] = "name";
//     vars[1] = "James";
//     rendered = render(tmpl, 2, vars);
//     free(vars);

//     if (rendered) {
//         printf("%s\n", rendered);
//     }
//     free_environment(env);
//     return 0;
// }