jinja2-c
========

C Bindings for Jinja 2 (so any language can use it)

Goals
=====

I really wanted to have a Rust binding for jinja 2 (and some day a native Rust implementation) 
so users can use Jinja 2 in my Rust webframework, but sadly I was having issues working with Python from Rust.
I decided I would make C bindings for the Python code and then I can call the c code from Rust.
I understand that it isn't the most elegant solution out there, but I want a templating language that 
I can use right now, and later I can make a full implementation in Rust.

Current State
=============

Well... it can render a template! Hurray! I will be adding more features to the bindings as needed.
The next step will be to make a library so other languages can call the code.
