from pygments import token
from pygments.lexer import RegexLexer
from sphinx.highlighting import lexers


class TitleFormatting(RegexLexer):
    name = "foobar2000 title formatting"
    aliases = ["fb2k"]
    filenames = []

    tokens = {
        "root": [
            (r" .*", token.Text),
            (r"$[a-z]+", token.Name.Function),
            (r"%[a-z]+%", token.Name.Variable),
        ]
    }


lexers["fb2k"] = TitleFormatting()
