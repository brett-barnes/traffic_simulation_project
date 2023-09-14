#include <iostream>
#include <random>

int main()
{
    std::mt19937 randomNumberGenerator; // Mersenne twister
    std::uniform_real_distribution<double> rand_double(0,1);

    //randomNumberGenerator.seed(39);
    randomNumberGenerator.seed(391);

    for (int i = 0; i < 10; i++)
    {
        // generate a uniform(0,1) psuedo-random number
        double randNum = rand_double(randomNumberGenerator); 

        // check which range within (0,1) the number falls
        if (randNum < 0.7)
            std::cout << "Car" << std::endl;
        else if (randNum < 0.9)
            std::cout << "SUV" << std::endl;
        else
            std::cout << "Truck" << std::endl;
    }

    for (int i = 0; i < 10; i++)
    {
        // this will print _exactly_ the same number on every
        // iteration because we are resetting the seed each
        // time (jumping into the random-number circle at the
        // same spot) -- _not_ what you want to do in your project
        randomNumberGenerator.seed(391);
        double randNum = rand_double(randomNumberGenerator);
        std::cout << randNum << std::endl;
    }

    return 0;
}
