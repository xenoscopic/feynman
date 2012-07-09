        gsl_monte_miser_state *s = gsl_monte_miser_alloc($integral.n_dimensions);
        gsl_monte_miser_integrate(&G, 
                                  lower_bounds, 
                                  upper_bounds, 
                                  $integral.n_dimensions, 
                                  n_calls, 
                                  r, 
                                  s, 
                                  &result, 
                                  &_error);
        gsl_monte_miser_free(s);
