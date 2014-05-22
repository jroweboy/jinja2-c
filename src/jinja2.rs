//! A safe interface to the Jinja 2 Python Library
//! As of writing, I could not get bindgen to build Python bindings for Rust,
//! so I instead built a small c wrapper and this library is a safe wrapper around the 
//! c library that interfaces with jinja2
use jinja2_c;
use std::str::raw::from_c_str;
use std::ptr;
use collections::HashMap;
use libc::{c_void, c_char};

pub struct Environment {
    env: *mut c_void,
}

pub struct Template {
    tmpl: *mut c_void,
}

impl Environment {
    /// Creates a new Environment that wraps the c calls needed to interact with the Jinja2 Environment
    ///
    /// Takes in a `str` for the absolute path to the template directory
    /// TODO? Refactor the C Library to allow any Loader and not just FileSystemLoader
    pub fn new(template_dir: &str) -> Environment {
        let env = unsafe { jinja2_c::init_environment(template_dir.to_c_str().unwrap()) };
        Environment {
            env: env,
        }
    }

    pub fn get_template(&self, template_name: &str) -> Template {
        let tmpl = unsafe { jinja2_c::get_template(self.env, template_name.to_c_str().unwrap()) };
        Template {
            tmpl: tmpl,
        }
    }

    pub fn list_templates(&self) {
        unsafe { 
            jinja2_c::list_templates(self.env);
        }
    }
}

impl Drop for Environment {
    fn drop(&mut self) {
        unsafe {
            jinja2_c::free_environment(self.env);
        }
    }
}

impl Template {
    pub fn render(&self, vars: Option<HashMap<~str, ~str>>) -> Option<~str> {
        let retval = match vars {
            None => unsafe { jinja2_c::render(self.tmpl, 0, ptr::null::<*mut c_char>() as *mut *mut c_char) },
            Some(s) => {
                // convert the hashmap to char*
                let mut vec = Vec::new();
                let len = s.len() * 2;
                for (ref key, ref val) in s.iter() {
                    unsafe {
                        vec.push(key.to_c_str().unwrap());
                        vec.push(val.to_c_str().unwrap());
                    }
                }
                unsafe { 
                    jinja2_c::render(self.tmpl, len as i32, vec.as_slice().as_ptr() as *mut *mut c_char) 
                }
            }
        };
        if retval.is_null() {
            None
        } else {
            Some(unsafe{ from_c_str(retval as *c_char) })
        }
    }
}