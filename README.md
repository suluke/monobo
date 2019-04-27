# cli_args
A header-only, declarative command line argument library for C++17.

This library is designed to scale across various project sizes.
To get started as quickly as possible, simply include the `cli_args.h` header.
`cli_args` should provide you with all the functionality you need to get started.
If you find out later that you want to alter some of `cli_args`' default behavior the lib's design makes this as easy as possible.
The declarative nature of how options are defined allows allows them to be treated completely independently from the parsing process.
The parser allows for a high degree of customization and can even be switched out entirely.

## Goals
* Declarative command line definition
* Type safety
* Automatic documentation (help dialog) generation
* Header-only implementation
* Easy customization and extension

## Non-Goals
* Windows-style option parsing ('/X /Y' instead of '-X -Y') - maybe 
  supported in a future version, though.
* Short vs long option differentiation (especially collapsing of
  multiple short options into one, e.g. '-abc' = '-a -b -c')
* Provide defaults for what can trivially be implemented by the user.
  E.g. default options like --help or --version.

## Features
* Declarative configuration of the option parser using option variables.
* Support for scalar and aggregate options.
* Support for '-opt=val' syntax
* Support for global or local option definitions.
  Local options will unregister automatically when going out of scope.
* Support for option namespaces/categories ("AppTag") to avoid
  option pollution from global options in different compilation units.
* Easy extendability for custom types by specializing `cli_args::detail::CliLibCfgStd::parse`.
  Alternatively, implement your own option types, only relying on
  the CliOptConcept interface.
* Automatic help text generation (`PrintHelp`).
* Support for 'stacked' option parsing, i.e. stopping parsing at some
  point to continue with a different parsing configuration.

## Brief Overview
* `cl::opt` for single-valued options
* `cl::list` for multi-valued options
* `cl::name` to specify names for an option
  - No name or "": Handle positional arguments
  - Multiple option names are possible
* `cl::meta` for describing value roles (used in `printHelp`)
* `cl::desc` for option description (used in `printHelp`)
* `cl::init` for initial option values
* `cl::required` to force users to provide a value
* `cl::storage` to re-use existing variables for option storage
* `cl::terminal` to stop the parser after the corresponding option
  has been parsed.
* `cl::app` to specify the application the option is intended for

## Technical Details
Declarativeness and header-only are two goals that are quite difficult to achieve at the same time.
Especially if we want to allow declaring options across multiple translation units (TUs).
Command line arguments parsing happens in a centralized place which needs to be aware of all available command line options before parsing can start.
With multiple TUs at play, it is up to the options to register with the central parsing procedure _before_ the parsing process begins.
Since option parsing is usually one of the first things a program does this essentially means that registration should happen even before `main` is called.
Registration cannot happen during compilation due to the multiple TU requirement.
Neither can we hope to achieve registration during link-time because this is a C++ library.
The only place left to perform option self-registration with the cli argument parser is therefore during program startup.
How could this work?
The most obvious idea is to have a global container variable that options insert themselves into during program startup.
However, this comes with two problems:
First, global variables are EW!
Secondly, we want this to be a _header-only_ library.
And globals cannot be defined in a header (c.f. One Definition Rule, ODR).
Or can they?
There exists a workaround that allows us to achieve something very similar:
It's static local variables in inline (as in non-ODR-affected) functions.
Quoting [the C++17 standard](https://github.com/cplusplus/draft/blob/c++17/source/declarations.tex#L1015):
> A `static` local variable in an inline
> function with external linkage always refers to the same object.

Furthermore, we can even improve upon the encapsulation of this singleton object by using a template function to hold our static local variable.
[This answer on StackOverflow](https://stackoverflow.com/a/994428/1468532) goes more into detail. 
