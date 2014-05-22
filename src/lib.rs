#![crate_id = "rustjinja2#0.0"]

#![comment = "Rust Bindings for Jinja2"]
#![license = "MIT/ASL2"]
#![crate_type = "dylib"]
#![crate_type = "rlib"]

#![doc(html_root_url = "http://www.rust-ci.org/jroweboy/jinja2-c/doc")]

// TODO make sure that this is uncommented before committing
//#![deny(missing_doc)]
#![feature(globs)]
#![feature(phase)]
#[phase(syntax, link)] extern crate log;
extern crate libc;
extern crate collections;

pub use jinja2::{Environment, Template};

mod jinja2_c;
pub mod jinja2;
