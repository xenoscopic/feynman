        //Do an initial warm-up
        gsl_monte_vegas_state *s = gsl_monte_vegas_alloc($integral.n_dimensions);
        gsl_monte_vegas_integrate(&G, 
                                  lower_bounds, 
                                  upper_bounds, 
                                  $integral.n_dimensions, 
                                  10000, 
                                  r, 
                                  s,
                                  &result,
                                  &_error);
        //Now actually converge on a result
        do
        {
            gsl_monte_vegas_integrate(&G, 
                                      lower_bounds, 
                                      upper_bounds, 
                                      $integral.n_dimensions, 
                                      n_calls/5, 
                                      r, 
                                      s,
                                      &result, 
                                      &_error);
        }
        while(fabs(gsl_monte_vegas_chisq(s) - 1.0) > 0.5);
        gsl_monte_vegas_free(s);
