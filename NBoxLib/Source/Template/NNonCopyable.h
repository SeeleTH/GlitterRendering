#pragma once

class NNonCopyable
{
private:
    NNonCopyable(const NNonCopyable& x);
    NNonCopyable& operator=(const NNonCopyable& x);
public:
    NNonCopyable() {}; // Default constructor  
};