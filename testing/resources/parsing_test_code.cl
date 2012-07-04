float test_function_1(float x, float y)
{
    //Cylinder with height 1.0 and radius
    //1.0 with bottom face's center at
    //the origin.
    float r = sqrt(pow(x, 2) + pow(y, 2));
    if(r <= 1.0)
    {
        //Inside the unit circle
        return 1.0;
    }

    //Outside the unit circle
    return 0.0;
}

int test_function_2(int a, bool b, float c, double d)
{
    if(a > 2)
    {
        return 5;
    }
    else if(b)
    {
        return 4;
    }
    else if(c < 1.0)
    {
        return 3;
    }
    else if(d < 2.0)
    {
        return 2;
    }
    return 1;
}

void test_function_3(double N, bool b, char c, int data)
{

}
