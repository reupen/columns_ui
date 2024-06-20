import os
from datetime import date

from pygments.lexer import RegexLexer, bygroups
from pygments.token import Comment, Name, Punctuation, Text
from sphinx.highlighting import lexers


class TitleFormatting(RegexLexer):
    name = "foobar2000 title formatting"
    aliases = ["fb2k"]
    filenames = []

    tokens = {
        "root": [
            (r"//.*?$", Comment.Single),
            (r"(\$\w+)(\()", bygroups(Name.Function, Punctuation), "function"),
            (r"%[\w]+%", Name.Variable),
            (r"[(),]", Punctuation),
            (r".", Text),
        ],
        "function": [
            (r"\)", Punctuation, "#pop"),
            (r"(\$\w+)(\()", bygroups(Name.Function, Punctuation), "#push"),
            (r"%[\w]+%", Name.Variable),
            (r"[(),]", Punctuation),
            (r"[^$%(),]+", Text),
        ],
    }


lexers["fb2k"] = TitleFormatting()


# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html


# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))

read_the_docs_build = os.environ.get("READTHEDOCS", None) == "True"

# -- Project information -----------------------------------------------------

project = "Columns UI"
copyright = f"Reupen Shah {date.today().year}"
author = "Reupen Shah"


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    "myst_parser",
]

myst_enable_extensions = []

myst_heading_anchors = 2

# Tell sphinx what the primary language being documented is.
primary_domain = "std"

# Tell sphinx what the pygments highlight language should be.
highlight_language = "fb2k"

# Add any paths that contain templates here, relative to this directory.
templates_path = ["_templates"]

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = []


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = "takao"

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = []
