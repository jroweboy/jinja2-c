jinja2-c
========

C Bindings for Jinja 2 (so any language can use it)

As an example of using the c bindings, I have also baked in a rust binding for the c library

There is some commented out sample code in the file src/jinja2.c 

Goals
=====

Reimplement as many features in Jinja2 in a C binding and also a Rust binding.

Current State
=============

The C library can be used to render a template and the Rust bindings for the C bindings are complete