
/**
 * fn init_environment(const char* template_dir) -> void*
 *
 * Parameters--
 * takes in a string of the absolute path to the template directory
 *
 * Returns--
 * Environment as a void* which is actually a PyObject* to the environment object created in python
 *
 * Notes--
 * I used a void* since I didn't want the user to have to import Python.h
 */
void* init_environment(const char*);

/**
 * fn render(void *template, int count, char** key_val_pairs) -> char* rendered_string
 * Calls the render function on this template and returns the rendered string. 
 * Do not call free on the rendered string!
 *
 * Parameters--
 * int count, the number of string arguments provided to that function
 * var_args must be of type char *
 *
 * Returns--
 * char* the rendered string
 *
 * ex: 
 * render(tmpl, 2, ["name", "James"]);
 * render(tmpl, 4, ["name", "James", "age", "24"]);
 * fails if you pass in an odd number for count
 */
char* render(void *, int, char**);

/**
 * fn free_environment(void* environment) -> void
 * 
 * Cleans up the Python environment and frees any memory still in use
 *
 * Parameters--
 * void* environment, which should be received from the init_environment function
 */
void free_environment(void*);

/** 
 * fn get_template(void* environment, const char* template_name) -> void *template
 * 
 * Parameters--
 * environment that has the template you wish to return
 * template_name is the file name of the template
 *
 * Returns--
 * void* representing the PyObject* for this template
 */
void* get_template(void*, const char*);

/**
 * fn list_templates(void* environment) -> void
 * Prints a list of the available templates to stdout. 
 * Maybe in the future, instead of printing, it should return a char** of all the templates?
 * If that is useful, I will rename this print templates, and make one to return the list
 *
 * Parameters--
 * environment received from the init_environment function
 */
void list_templates(void* env);
