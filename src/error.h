#ifndef __JR_ERROR_H__
#define __JR_ERROR_H__
//an exception class that is thrown for every exception thrown in
//the program.

#include <sstream>
#include <string>

using namespace std;

class Error
{
    public:
    Error(string errorString = "");
    Error(const Error& copy);
    ~Error();

    stringstream errorStringStream;

    friend Error&      operator<<(Error& error, string add);
    friend Error&      operator<<(Error& error, char add);
    friend ostream&    operator<<(ostream& out, Error& error);
};

Error&      operator<<(Error& error, string add);
Error&      operator<<(Error& error, int i);
ostream&    operator<<(ostream& out, Error& error);

#endif
