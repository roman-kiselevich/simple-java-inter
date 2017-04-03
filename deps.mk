buffer.o: buffer.c buffer.h soui.h boolean.h qoui.h itrerror.h restab.h
lexer.o: lexer.c lexer.h token.h restab.h buffer.h itrerror.h boolean.h \
 kwrecognizer.h
restab.o: restab.c restab.h itrerror.h
itrerror.o: itrerror.c itrerror.h restab.h
qoui.o: qoui.c qoui.h boolean.h restab.h
soui.o: soui.c soui.h boolean.h restab.h
symtab.o: symtab.c symtab.h boolean.h ifj16funcs.h restab.h itrerror.h
ilist.o: ilist.c ilist.h exprtree.h token.h symtab.h boolean.h \
 ifj16funcs.h restab.h
exprtree.o: exprtree.c exprtree.h token.h symtab.h boolean.h ifj16funcs.h \
 restab.h itrerror.h
prectab.o: prectab.c prectab.h exprtree.h token.h symtab.h boolean.h \
 ifj16funcs.h restab.h parser.h lexer.h itrerror.h
kwrecognizer.o: kwrecognizer.c kwrecognizer.h boolean.h token.h restab.h
parser.o: parser.c ilist.h exprtree.h token.h symtab.h boolean.h \
 ifj16funcs.h parser.h lexer.h restab.h prectab.h itrerror.h soexpitems.h \
 cmptcheck.h
soexpitems.o: soexpitems.c soexpitems.h symtab.h boolean.h ifj16funcs.h \
 exprtree.h token.h ilist.h restab.h cmptcheck.h itrerror.h
cmptcheck.o: cmptcheck.c cmptcheck.h symtab.h boolean.h ifj16funcs.h \
 exprtree.h token.h itrerror.h
inter.o: inter.c inter.h restab.h symtab.h boolean.h ifj16funcs.h \
 itrerror.h ilist.h exprtree.h token.h fcallstack.h soexpitems.h
fcallstack.o: fcallstack.c fcallstack.h ilist.h exprtree.h token.h \
 symtab.h boolean.h ifj16funcs.h restab.h itrerror.h
ifj16funcs.o: ifj16funcs.c ifj16funcs.h symtab.h boolean.h restab.h
