# feynman

An experimental Monte Carlo integration framework that allows integrands to be
evaluated on either a CPU (using GSL) or a GPU via the same integration API.
The original goal was to implement VEGAS GPU integration for Matrix Element
Method analyses, but sadly I ran out of time to make that a reality...  Plain
Monte Carlo integration *is* implemented on the GPU, and even works for
some sample integrands, but VEGAS is not.

This code is likely not fit for any use, but might be a useful reference in the
future.
