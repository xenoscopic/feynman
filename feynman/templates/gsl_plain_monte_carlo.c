        gsl_monte_plain_state *s = gsl_monte_plain_alloc($integral.n_dimensions);
        gsl_monte_plain_integrate(&G, 
                                  lower_bounds, 
                                  upper_bounds, 
                                  $integral.n_dimensions, 
                                  n_calls, 
                                  r, 
                                  s, 
                                  &result, 
                                  &_error);
        gsl_monte_plain_free(s);
