#ifndef SWITCHES_H
#define SWITCHES_H

#include <ostream>
#include <cstdlib>
#include <cstring>

// ======================================================================

class option {
  public:
    class argument {
        unsigned refcount;
      protected:
        const char* name;
        const char* doc;
        virtual ~argument();
      public:
        argument(const char* name, const char* doc);

        void show(std::ostream& s, unsigned &col) const;
        virtual void show_current(std::ostream &out, unsigned col2) const;

        // Write to stderr and exits, on error
        virtual void pass(char L, const char* arg) = 0;

        // Document legal values if necessary
        virtual void document(std::ostream& out, unsigned col2) const;

        inline argument* link() {
          ++refcount;
          return this;
        }
        void unlink() {
          --refcount;
          if (0==refcount) delete this;
        }

        inline bool name_hidden() const {
          return (name[0] == ' ');
        }
    };

  private:  
    unsigned numargs;
    argument** A;
    const char* doc;
    
  public:
    option(const char* doc, unsigned numargs, ...);
    virtual ~option();

    unsigned process(char letter, const char** argp);

    /**
      Document this option.
    */
    void document(char letter, std::ostream& out, unsigned indent, unsigned col2) const;

    virtual void activate(char letter); 

    virtual void show_current(std::ostream& out, unsigned col2) const;
};

// ======================================================================

class letter_option : public option {
    char& state;
    char actletter;
  public:
    letter_option(char &state, const char* doc);
    virtual ~letter_option();

    virtual void activate(char letter);
    virtual void show_current(std::ostream& out, unsigned col2) const;
};

// ======================================================================

class string_arg : public option::argument {
    const char* &state;
  public:
    string_arg(const char* name, const char* &st, const char* doc);
    virtual ~string_arg();

    virtual void show_current(std::ostream &out, unsigned col2) const;
    virtual void pass(char L, const char* arg);
};

// ======================================================================

class enum_arg : public option::argument {
  public:
    class value {
        unsigned refcount;
        const char* valstr;
        const char* doc;
      
        friend class enum_arg;
      public:
        value() {
          refcount=1;
        }

        value(const char* v, const char* d) {
          refcount=1;
          set(v, d);
        }

        const char* get_str() const { 
          return valstr;
        }

        inline void set(const char* v, const char* d) {
          valstr = v;
          doc = d;
        }

        inline value* link() {
          ++refcount;
          return this;
        }
        inline void unlink() {
          --refcount;
          if (0==refcount) delete this;
        }
    };

  private:
    unsigned &state;
    unsigned numvals;
    value** valstrings;
  public:
    enum_arg(const char* name, unsigned &st, const char* doc, unsigned numvals, ...);
    virtual ~enum_arg();
    virtual void show_current(std::ostream &out, unsigned col2) const;
    virtual void pass(char L, const char* arg);
    virtual void document(std::ostream& out, unsigned col2) const;
};

// ======================================================================

class unsigned_arg : public option::argument {
    unsigned &state;
  public:
    unsigned_arg(const char* name, unsigned &U, const char* doc);
    virtual ~unsigned_arg();

    virtual void show_current(std::ostream &out, unsigned col2) const;
    virtual void pass(char L, const char* arg);
};

// ======================================================================

class signed_arg : public option::argument {
    int &state;
  public:
    signed_arg(const char* name, int &I, const char* doc);
    virtual ~signed_arg();

    virtual void show_current(std::ostream &out, unsigned col2) const;
    virtual void pass(char L, const char* arg);
};

// ======================================================================

class optman {
  public:
    optman(char help, const char* usage);
    ~optman();

    inline void set_col1(unsigned char col1) {
      col1pos = col1;
    }
    inline void set_col2(unsigned char col2) {
      col2pos = col2;
    }

    bool add_option(char letter, option*);

    /**
      Process command line arguments until no more matches.
      Returns index of first non-processed argument.
    */
    int parse_switches(int argc, const char** argv);

    void show_usage(std::ostream &out, const char* exe) const;

  private:
    void document_letter(std::ostream &out, char L) const;

  private:
    option** optarray; 
    const char* usage_header;
    char help_letter;
    unsigned char col1pos, col2pos;
};

#endif
