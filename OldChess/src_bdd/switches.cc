
#include <cstring>
#include <iostream>
#include <stdarg.h>
#include "switches.h"


// #define DEBUG


// ======================================================================

inline void write_spaces(std::ostream& out, unsigned i)
{
  for (; i; --i) {
    out.put(' ');
  }
}

inline void writestr(std::ostream& out, unsigned indent, const char* D)
{
  for (; *D; D++) {
    out.put(*D);
    if ('\n' == *D) write_spaces(out, indent);
  }
}

// ======================================================================

option::argument::argument(const char* n, const char* d)
{
  name = n;
  doc = d;
  refcount=1;
}

option::argument::~argument()
{
}

void option::argument::show(std::ostream& s, unsigned &col) const
{
  const char* p = name;
  if (' ' == *p) ++p;
  while (*p) {
    ++col;
    s.put(*p++);
  }
}

void option::argument::show_current(std::ostream&, unsigned) const
{
}

void option::argument::document(std::ostream& out, unsigned col2) const
{
  if (doc) {
    out << "\n";
    write_spaces(out, col2);
    if (! name_hidden()) {
      out << name << " ";
    }
    writestr(out, col2, doc);
  }
}

// ======================================================================

option::option(const char* d, unsigned na, ...)
{
  doc = d;
  numargs = na;
  if (na) {
    A = new argument* [numargs];
    va_list argp;
    va_start(argp, na);
    for (unsigned i=0; i<numargs; ++i) {
      A[i] = va_arg(argp, argument*);
    }
    va_end(argp);
  } else {
    A = 0;
  }
}

option::~option()
{
  for (unsigned i=0; i<numargs; ++i) {
    if (A[i]) A[i]->unlink();
  }
  delete[] A;
}

unsigned option::process(char letter, const char** argp)
{
  activate(letter);
  for (unsigned u=0; u<numargs; u++) {
    if (A[u]) A[u]->pass(letter, argp[u]);
  }
  return numargs;
}

void option::document(char letter, std::ostream& out, unsigned ind, unsigned col2) const
{
  write_spaces(out, ind);

  unsigned col1 = ind;
  out << '-' << letter;
  col1 += 2;
  for (unsigned u=0; u<numargs; u++) {
    if (A[u]) {
      out << " ";
      col1++;
      A[u]->show(out, col1);
    }
  }

  if (col1 < col2) {
    write_spaces(out, col2-col1);
  } else {
    out << "\n";
    write_spaces(out, col2);
  }

  writestr(out, col2, doc);
  for (unsigned u=0; u<numargs; u++) {
    if (A[u]) {
      if (!A[u]->name_hidden()) out << "\n";
      A[u]->document(out, col2);
      A[u]->show_current(out, col2);
    }
  }
  show_current(out, col2);
  out << "\n";
}

void option::activate(char)
{
}

void option::show_current(std::ostream&, unsigned) const
{
}

// ======================================================================

letter_option::letter_option(char &st, const char* doc)
 : option(doc, 0), state(st)
{
  actletter = 0;
}

letter_option::~letter_option()
{
}

void letter_option::activate(char letter)
{
  state = letter;
  actletter = letter;
}

void letter_option::show_current(std::ostream& out, unsigned col2) const
{
  if (actletter && actletter == state) {
    out << "\n";
    write_spaces(out, col2);
    out << "Currently set.";
  }
}

// ======================================================================

string_arg::string_arg(const char* nam, const char* &st, const char* dc)
 : argument(nam, dc), state(st)
{
}

string_arg::~string_arg()
{
}

void string_arg::show_current(std::ostream &out, unsigned col2) const
{
  if (state) {
    out << "\n";
    write_spaces(out, col2);
    if (name_hidden())  out << "Currently set to " << state;
    else                out << name << " is currently: " << state;
  }
}

void string_arg::pass(char L, const char* arg)
{
  if (0==arg) {
    std::cerr << "Error: string expected for " << name << " after -" << L << "\n";
    exit(1);
  }
  state = arg; 
}

// ======================================================================

enum_arg::enum_arg(const char* nam, unsigned &st, const char* dc, unsigned nv, ...)
 : argument(nam, dc), state(st)
{
  numvals = nv;
  if (numvals) {
    valstrings = new value*[numvals];
    va_list argp;
    va_start(argp, nv);
    for (unsigned u=0; u<numvals; ++u) {
      valstrings[u] = va_arg(argp, value*);
    }
    va_end(argp);
  } else {
    valstrings = 0;
  }
}

enum_arg::~enum_arg()
{
  for (unsigned u=0; u<numvals; ++u) {
    if (valstrings[u]) valstrings[u]->unlink();
  }
  delete[] valstrings;
}

void enum_arg::show_current(std::ostream &out, unsigned col2) const
{
  out << "\n";
  write_spaces(out, col2);
  if (name_hidden())  out << "Currently set to ";
  else                out << name << " is currently: ";
  out << valstrings[state]->valstr;
}

void enum_arg::pass(char L, const char* arg)
{
  if (0==arg) {
    std::cerr << "Error: string expected for " << name << " after -" << L << "\n";
    exit(1);
  }
  for (unsigned u=0; u<numvals; ++u) {
    if (0==strcmp(valstrings[u]->valstr, arg)) {
      state = u;
      return;
    }
  }
  std::cerr << "Error: unknown value " << arg << " for " << name << " after -" << L << "\n";
  exit(1);
}

void enum_arg::document(std::ostream& out, unsigned col2) const
{
  argument::document(out, col2);

  unsigned maxlen = 0;
  for (unsigned u=0; u<numvals; ++u) {
    unsigned ulen = static_cast <unsigned>(strlen(valstrings[u]->valstr));
    if (ulen > maxlen) maxlen = ulen;
  }

  out << "\n";
  write_spaces(out, col2);
  out << "Options";
  if (! name_hidden()) out << " for " << name;
  out << " are:";

  for (unsigned u=0; u<numvals; ++u) {
    out << "\n";
    unsigned ulen = static_cast <unsigned>(strlen(valstrings[u]->valstr));
    write_spaces(out, col2+2);
    out << valstrings[u]->valstr;
    write_spaces(out, 2 + maxlen - ulen);
    writestr(out, col2+maxlen+4, valstrings[u]->doc);
  }
}

// ======================================================================

unsigned_arg::unsigned_arg(const char* nam, unsigned &U, const char* dc)
 : argument(nam, dc), state(U)
{
}

unsigned_arg::~unsigned_arg()
{
}

void unsigned_arg::show_current(std::ostream &out, unsigned col2) const
{
  out << "\n";
  write_spaces(out, col2);
  if (name_hidden())  out << "Currently set to " << state;
  else                out << name << " is currently: " << state;
}

void unsigned_arg::pass(char L, const char* arg)
{
  unsigned ans = 0;
  try {
    if (0==arg) throw 1;
    while (*arg) {
      if (*arg < '0') throw 2;
      if (*arg > '9') throw 3;
      ans *= 10;
      ans += unsigned(*arg - '0');
      arg++;
    }
    state = ans;
  }
  catch (int) {
    std::cerr << "Error: integer expected for " << name << " after -" << L << "\n";
    exit(1);
  }
}

// ======================================================================

signed_arg::signed_arg(const char* nam, int &I, const char* dc)
 : argument(nam, dc), state(I)
{
}

signed_arg::~signed_arg()
{
}

void signed_arg::show_current(std::ostream &out, unsigned col2) const
{
  out << "\n";
  write_spaces(out, col2);
  if (name_hidden())  out << "Currently set to " << state;
  else                out << name << " is currently: " << state;
}

void signed_arg::pass(char L, const char* arg)
{
  try {
    if (0==arg) throw 1;
    const unsigned start = ('-' == arg[0]) ? 1 : 0;
    for (unsigned i=start; arg[i]; i++) {
      if (arg[i] < '0') throw 2;
      if (arg[i] > '9') throw 3;
    }
    state = static_cast <int>(atol(arg));
  }
  catch (int) {
    std::cerr << "Error: integer expected for " << name << " after -" << L << "\n";
    exit(1);
  }
}


// ======================================================================

optman::optman(char help, const char* usage)
{
  help_letter = help;
  usage_header = usage;
  set_col1(4);
  set_col2(15);

  optarray = new option*[128];
  for (unsigned i=0; i<128; i++) optarray[i] = 0;
}

optman::~optman()
{
  for (unsigned i=0; i<128; i++) {
    delete optarray[i];
  }
  delete[] optarray;
}

bool optman::add_option(char letter, option* o)
{
  if (0==o)                       return false;
  if (letter<0)                   return false;
  // if (letter > 127)               return false;
  if (letter == help_letter)      return false;
  if (optarray[unsigned(letter)]) return false;
  optarray[unsigned(letter)] = o;
  return true;
}

int optman::parse_switches(int argc, const char** argv)
{
  int i;
  for (i=1; i<argc; ) {
#ifdef DEBUG
    std::cerr << "Checking argument: '" << argv[i] << "'\n";
#endif
    if (0   == argv[i][0]) return i;
    if ('-' != argv[i][0]) return i;
    if (0   == argv[i][1]) return i;
    if (0   != argv[i][2]) return i;
    const char letter = argv[i][1];
    if (help_letter == letter) {
      show_usage(std::cerr, argv[0]);
      exit(1);
    }
    option* o = optarray[unsigned(letter)];
    if (!o) return i;
    ++i;
    i += o->process(letter, argv+i);
  }
  return i;
}

void optman::show_usage(std::ostream &out, const char* exe) const
{
  const char* baseexe = exe;
  for ( ; *exe; exe++) {
    if ('/' == *exe) {
      baseexe = exe+1;
    }
  }
  out << "Usage: " << baseexe << " " << usage_header;
  out << "\nOptions:\n";

  for (char u=0; u<127; ++u) {
    if ((u>='a') && (u<='z')) continue;
    if ((u>='A') && (u<='Z')) {
      document_letter(out, u+32);
    } 
    document_letter(out, u);
  }
}

void optman::document_letter(std::ostream &out, char L) const
{
  if (help_letter == L) {
    write_spaces(out, col1pos);
    out << '-' << help_letter;
    write_spaces(out, col2pos-(col1pos+2));
    writestr(out, col2pos, "This help");
    out << "\n";
  } else if (optarray[int(L)]) {
    optarray[int(L)]->document(L, out, col1pos, col2pos);
  } else {
    return;
  }
  out << "\n";
}

