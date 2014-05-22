extern crate rustjinja2;
extern crate collections;

use collections::HashMap;
use rustjinja2::{Environment, Template};

fn main() {
    let env = Environment::new("/home/jrowe7/slf/jinja2-c/test_template");
    let tmpl = env.get_template("index.html");
    let mut vars = HashMap::new();
    vars.insert("name".to_owned(), "James".to_owned());
    println!("{}", tmpl.render(Some(vars)).expect("Render failed?"));
}