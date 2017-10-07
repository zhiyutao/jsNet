#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../dev/cpp/Network.cpp"
#include <vector>
#include <algorithm>

bool doublesAreEqual(double a, double b) {
    return a==b || std::abs(a-b)<std::abs(std::min(a,b))*std::numeric_limits<double>::epsilon();
}

bool moreOrLessEqual (double a, double b, int decimalPlaces) {
    return round(a * pow(10, decimalPlaces))/decimalPlaces == round(b * pow(10, decimalPlaces))/decimalPlaces;
}

double standardDeviation (std::vector<double> arr) {
    double avg = 0;

    for (int i=0; i<arr.size(); i++) {
        avg += arr[i];
    }

    avg /= arr.size();

    for (int i=0; i<arr.size(); i++) {
        arr[i] = pow(arr[i] - avg, 2);
    }

    double var = 0;

    for (int i=0; i<arr.size(); i++) {
        var += arr[i];
    }

    return sqrt(var / arr.size());
}

TEST_CASE("standardDeviation") {
    std::vector<double> vals = {3,5,7,8,5,25,8,4};
    REQUIRE( standardDeviation(vals) == 6.603739470936145 );
}


/* Network */
TEST_CASE("Network::newNetwork - Appends a new instance to the Network::instances vector, returning instance index") {
    REQUIRE( Network::netInstances.size() == 0 );
    Network::newNetwork();
    REQUIRE( Network::netInstances.size() == 1 );
    Network::newNetwork();
    REQUIRE( Network::netInstances.size() == 2 );
}
TEST_CASE("Network::newNetwork - Returns the index of the newly created instance") {
    Network::deleteNetwork();
    REQUIRE( Network::newNetwork()==0 );
    REQUIRE( Network::newNetwork()==1 );
    REQUIRE( Network::newNetwork()==2 );
}

TEST_CASE("Network::getInstance - Returns the correct Network instance") {
    Network::netInstances[0]->learningRate = (float) 1;
    Network::netInstances[1]->learningRate = (float) 2;
    Network::netInstances[2]->learningRate = (float) 3;
    REQUIRE( Network::getInstance(0)->learningRate == (float) 1 );
    REQUIRE( Network::getInstance(1)->learningRate == (float) 2 );
    REQUIRE( Network::getInstance(2)->learningRate == (float) 3 );
}

TEST_CASE("Network::deleteNetwork - Deletes a network instance when given an index") {
    REQUIRE( Network::netInstances.size() == 3 );
    REQUIRE( Network::netInstances[1] != 0 );
    Network::deleteNetwork(1);
    REQUIRE(Network::netInstances[1] == 0);
}

TEST_CASE("Network::deleteNetwork - Deletes all network instances when no index is given") {
    REQUIRE( Network::netInstances.size() == 3 );
    Network::deleteNetwork();
    REQUIRE( Network::netInstances.size() == 0 );
}


TEST_CASE("Network::joinLayers - Assigns the network activation function to each layer") {
    Network::deleteNetwork();
    Network::newNetwork();
    Network::getInstance(0)->weightsConfig["limit"] = 0.1;
    Network::getInstance(0)->weightInitFn = &NetMath::uniform;
    Network::getInstance(0)->layers.push_back(new Layer(0, 3));
    Network::getInstance(0)->layers.push_back(new Layer(0, 3));
    Network::getInstance(0)->layers.push_back(new Layer(0, 3));
    Network::getInstance(0)->activation = &NetMath::sigmoid;

    REQUIRE( Network::getInstance(0)->layers[0]->activation != &NetMath::sigmoid );
    REQUIRE( Network::getInstance(0)->layers[1]->activation != &NetMath::sigmoid );
    Network::getInstance(0)->joinLayers();
    REQUIRE( Network::getInstance(0)->layers[0]->activation == &NetMath::sigmoid );
    REQUIRE( Network::getInstance(0)->layers[1]->activation == &NetMath::sigmoid );
}

TEST_CASE("Network::joinLayers - Assigns prevLayers to layers accordingly") {
    REQUIRE( Network::getInstance(0)->layers[1]->prevLayer == Network::getInstance(0)->layers[0] );
    REQUIRE( Network::getInstance(0)->layers[2]->prevLayer == Network::getInstance(0)->layers[1] );
}

TEST_CASE("Network::joinLayers - Assigns nextLayers to layers accordingly") {
    REQUIRE( Network::getInstance(0)->layers[0]->nextLayer == Network::getInstance(0)->layers[1] );
    REQUIRE( Network::getInstance(0)->layers[1]->nextLayer == Network::getInstance(0)->layers[2] );
}

// TEST_CASE("Network::joinLayers - Assigns fanIn and fanOut correctly") {
//     REQUIRE( Network::getInstance(0)->layers[0]->fanIn == -1 );
    // REQUIRE( Network::getInstance(0)->layers[0]->fanOut == 3 );
    // REQUIRE( Network::getInstance(0)->layers[1]->fanIn == 3 );
    // REQUIRE( Network::getInstance(0)->layers[1]->fanOut == 3 );
    // REQUIRE( Network::getInstance(0)->layers[2]->fanIn == 3 );
    // REQUIRE( Network::getInstance(0)->layers[2]->fanOut == -1 );
// }

TEST_CASE("Network::forward - Sets the first layer's neurons' activations to the input given") {

    std::vector<double> testInput = {1,2,3};

    REQUIRE(Network::getInstance(0)->layers[0]->neurons[0]->activation != 1);
    REQUIRE(Network::getInstance(0)->layers[0]->neurons[1]->activation != 2);
    REQUIRE(Network::getInstance(0)->layers[0]->neurons[2]->activation != 3);

    Network::getInstance(0)->forward(testInput);

    REQUIRE(Network::getInstance(0)->layers[0]->neurons[0]->activation == 1);
    REQUIRE(Network::getInstance(0)->layers[0]->neurons[1]->activation == 2);
    REQUIRE(Network::getInstance(0)->layers[0]->neurons[2]->activation == 3);
}


TEST_CASE("Network::forward - Returns a vector of activations in the last layer") {

    std::vector<double> testInput = {1,2,3};
    std::vector<double> returned = Network::getInstance(0)->forward(testInput);

    std::vector<double> actualValues;

    for (int i=0; i<3; i++) {
        actualValues.push_back(Network::getInstance(0)->layers[2]->neurons[i]->activation);
    }

    REQUIRE( returned == actualValues );
}


TEST_CASE("Network::resetDeltaWeights - Sets all the delta weights values to 0") {
    Network::deleteNetwork();
    Network::newNetwork();
    Network::getInstance(0)->weightsConfig["limit"] = 0.1;
    Network::getInstance(0)->weightInitFn = &NetMath::uniform;
    Network::getInstance(0)->layers.push_back(new Layer(0, 3));
    Network::getInstance(0)->layers.push_back(new Layer(0, 3));
    Network::getInstance(0)->layers.push_back(new Layer(0, 3));
    Network::getInstance(0)->joinLayers();

    for (int n=1; n<3; n++) {
        Network::getInstance(0)->layers[0]->neurons[n]->deltaWeights = {1,1,1};
        Network::getInstance(0)->layers[1]->neurons[n]->deltaWeights = {1,1,1};
    }

    Network::getInstance(0)->resetDeltaWeights();
    std::vector<double> expected = {0,0,0};

    for (int n=1; n<3; n++) {
        REQUIRE( Network::getInstance(0)->layers[1]->neurons[n]->deltaWeights == expected );
        REQUIRE( Network::getInstance(0)->layers[2]->neurons[n]->deltaWeights == expected );
    }
}

TEST_CASE("Network::applyDeltaWeights - Increment the weights by the delta weights") {
    Network::getInstance(0)->learningRate = 1;
    Network::getInstance(0)->updateFnIndex = 0;
    for (int n=1; n<3; n++) {
        Network::getInstance(0)->layers[1]->neurons[n]->weights = {1,1,1};
        Network::getInstance(0)->layers[2]->neurons[n]->weights = {2,2,2};
        Network::getInstance(0)->layers[1]->neurons[n]->deltaWeights = {1,2,3};
        Network::getInstance(0)->layers[2]->neurons[n]->deltaWeights = {4,5,6};
    }

    std::vector<double> expected1 = {2,3,4};
    std::vector<double> expected2 = {6,7,8};

    Network::getInstance(0)->applyDeltaWeights();

    for (int n=1; n<3; n++) {
        REQUIRE( Network::getInstance(0)->layers[1]->neurons[n]->weights == expected1 );
        REQUIRE( Network::getInstance(0)->layers[2]->neurons[n]->weights == expected2 );
    }
}

/* Layer */
TEST_CASE("Layer::assignNext - Assigns the given layer pointer to this layer's nextLayer") {
    Layer* l1 = new Layer(0, 1);
    Layer* l2 = new Layer(0, 1);

    l1->assignNext(l2);

    delete l1;
    delete l2;
}

TEST_CASE("Layer::assignPrev - Assigns the given layer pointer to this layer's prevLayer") {
    Layer* l1 = new Layer(0, 1);
    Layer* l2 = new Layer(0, 1);

    l2->assignPrev(l1);

    delete l1;
    delete l2;
}

TEST_CASE("Layer::init - Fills the layers' neurons vectors with as many neurons as the layers' sizes") {
    Layer* l1 = new Layer(0, 2);
    Layer* l2 = new Layer(0, 5);
    l2->assignPrev(l1);

    REQUIRE( l2->neurons.size() != 5 );
    l2->init(1);
    REQUIRE( l2->neurons.size() == 5 );

    delete l1;
    delete l2;
}

TEST_CASE("Layer::init - Sets the bias of every neuron to a number between -0.1 and 0.1") {
    Layer* l1 = new Layer(0, 2);
    Layer* l2 = new Layer(0, 5);
    l2->assignPrev(l1);
    l2->init(1);

    for (int n=0; n<5; n++) {
        // REQUIRE(l2->neurons[n]->bias == 0);
        REQUIRE(l2->neurons[n]->bias >= -0.1);
        REQUIRE(l2->neurons[n]->bias <= 0.1);
    }

    delete l1;
    delete l2;
}

TEST_CASE("Layer::init - Inits the neurons' weights vector with as many weights as there are neurons in the prev layer. (none in first layer)") {
    Layer* l1 = new Layer(0, 2);
    Layer* l2 = new Layer(0, 5);
    l2->assignPrev(l1);
    l1->init(0);
    l2->init(1);

    REQUIRE(l1->neurons[0]->weights.size() == 0);
    REQUIRE(l1->neurons[1]->weights.size() == 0);

    for (int n=0; n<5; n++) {
        REQUIRE(l2->neurons[n]->weights.size() == 2);
    }

    delete l1;
    delete l2;
}

TEST_CASE("Layer::forward - SETS the neurons' sum to their bias + weighted activations of last layer's neurons") {
    Layer* l1 = new Layer(0, 2);
    Layer* l2 = new Layer(0, 3);
    l2->assignPrev(l1);
    l1->init(0);
    l2->init(1);
    l2->activation = &NetMath::sigmoid;

    l1->neurons[0]->activation = 1;
    l1->neurons[1]->activation = 2;

    for (int n=0; n<3; n++) {
        l2->neurons[n]->weights = {1,2};
        l2->neurons[n]->bias = n;
    }

    Network::getInstance(0)->isTraining = false;
    Network::getInstance(0)->dropout = 1;
    l2->netInstance = 0;
    l2->forward();

    REQUIRE(l2->neurons[0]->sum == 5);
    REQUIRE(l2->neurons[1]->sum == 6);
    REQUIRE(l2->neurons[2]->sum == 7);

    // Check that it SETS it, and doesn't increment it
    l2->forward();

    REQUIRE(Network::getInstance(0)->isTraining==false);
    REQUIRE(l2->neurons[0]->sum == 5);
    REQUIRE(l2->neurons[1]->sum == 6);
    REQUIRE(l2->neurons[2]->sum == 7);

    delete l1;
    delete l2;
}

TEST_CASE("Layer::forward - Sets the layer's neurons' activation to the result of the activation function") {
    Layer* l1 = new Layer(0, 2);
    Layer* l2 = new Layer(0, 3);
    l2->assignPrev(l1);
    l1->init(0);
    l2->init(1);
    l2->activation = &NetMath::sigmoid;

    l1->neurons[0]->activation = 1;
    l1->neurons[1]->activation = 2;

    for (int n=0; n<3; n++) {
        l2->neurons[n]->weights = {1,2};
        l2->neurons[n]->bias = n;
    }

    Network::getInstance(0)->isTraining = false;
    Network::getInstance(0)->dropout = 1;
    l2->forward();

    REQUIRE(Network::getInstance(0)->isTraining==false);
    REQUIRE(l2->neurons[0]->activation == 0.9933071490757153);
    REQUIRE(l2->neurons[1]->activation == 0.9975273768433653);
    REQUIRE(l2->neurons[2]->activation == 0.9990889488055994);

    delete l1;
    delete l2;
}

TEST_CASE("Layer::forward - Sets the neurons' dropped value to true and activation to 0 if the net is training and dropout is set to 0") {
    Layer* l1 = new Layer(0, 2);
    Layer* l2 = new Layer(0, 3);
    l2->assignPrev(l1);
    l1->init(0);
    l2->init(1);
    l2->activation = &NetMath::sigmoid;

    l1->neurons[0]->activation = 1;
    l1->neurons[1]->activation = 2;

    for (int n=0; n<3; n++) {
        l2->neurons[n]->weights = {1,2};
        l2->neurons[n]->bias = n;
        l2->neurons[n]->sum = 0;
    }

    l2->netInstance = 0;
    Network::getInstance(0)->dropout = 0;
    Network::getInstance(0)->isTraining = true;
    l2->forward();

    REQUIRE(l2->neurons[0]->dropped);
    REQUIRE(l2->neurons[1]->dropped);
    REQUIRE(l2->neurons[2]->dropped);

    REQUIRE(l2->neurons[0]->activation == 0);
    REQUIRE(l2->neurons[1]->activation == 0);
    REQUIRE(l2->neurons[2]->activation == 0);

    REQUIRE(l2->neurons[0]->sum == 0);
    REQUIRE(l2->neurons[1]->sum == 0);
    REQUIRE(l2->neurons[2]->sum == 0);

    delete l1;
    delete l2;
}

TEST_CASE("Layer::forward - Does not set neurons to dropped if the net is not training") {
    Layer* l1 = new Layer(0, 2);
    Layer* l2 = new Layer(0, 3);
    l2->assignPrev(l1);
    l1->init(0);
    l2->init(1);
    l2->activation = &NetMath::sigmoid;

    l1->neurons[0]->activation = 1;
    l1->neurons[1]->activation = 2;

    for (int n=0; n<3; n++) {
        l2->neurons[n]->weights = {1,2};
        l2->neurons[n]->bias = n;
        l2->neurons[n]->activation = 0;
    }

    l2->netInstance = 0;
    Network::getInstance(0)->dropout = 1;
    Network::getInstance(0)->isTraining = false;
    l2->forward();

    REQUIRE( !l2->neurons[0]->dropped );
    // REQUIRE( !l2->neurons[1]->dropped );
    // REQUIRE( !l2->neurons[2]->dropped );

    REQUIRE(l2->neurons[0]->activation == 0.9933071490757153);
    REQUIRE(l2->neurons[1]->activation == 0.9975273768433653);
    REQUIRE(l2->neurons[2]->activation == 0.9990889488055994);

    delete l1;
    delete l2;
}


TEST_CASE("Layer::forward - Divides the activation values by the dropout") {
    Layer* l1 = new Layer(0, 2);
    Layer* l2 = new Layer(0, 3);
    l2->assignPrev(l1);
    l1->init(0);
    l2->init(1);
    l2->activation = &NetMath::sigmoid;

    l1->neurons[0]->activation = 1;
    l1->neurons[1]->activation = 2;

    for (int n=0; n<3; n++) {
        l2->neurons[n]->weights = {1,2};
        l2->neurons[n]->bias = n;
    }

    Network::getInstance(0)->dropout = 0.5;
    Network::getInstance(0)->isTraining = false;
    l2->forward();

    REQUIRE(Network::getInstance(0)->isTraining==false);
    REQUIRE(l2->neurons[0]->activation == 0.9933071490757153 * 2);
    REQUIRE(l2->neurons[1]->activation == 0.9975273768433653 * 2);
    REQUIRE(l2->neurons[2]->activation == 0.9990889488055994 * 2);

    delete l1;
    delete l2;
}

TEST_CASE("Layer::backward - Sets the neurons' errors to difference between their activations and expected values, when provided") {
    Layer* l1 = new Layer(0, 2);
    Layer* l2 = new Layer(0, 3);
    l2->assignPrev(l1);
    l1->init(0);
    l2->init(1);

    std::vector<double> expected = {1,2,3};

    l2->neurons[0]->activation = 0;
    l2->neurons[1]->activation = 1;
    l2->neurons[2]->activation = 0;

    l2->neurons[0]->dropped = false;
    l2->neurons[1]->dropped = false;
    l2->neurons[2]->dropped = false;

    l2->backward(expected);

    REQUIRE( l2->neurons[0]->error == 1 );
    REQUIRE( l2->neurons[1]->error == 1 );
    REQUIRE( l2->neurons[2]->error == 3 );

    delete l1;
    delete l2;
}

TEST_CASE("Layer::backward - Sets the neurons' derivatives to the activation prime of their sum, when no expected data is passed") {
    Layer* l1 = new Layer(0, 2);
    Layer* l2 = new Layer(0, 3);
    Layer* l3 = new Layer(0, 4);
    l2->assignPrev(l1);
    l2->assignNext(l3);
    l3->assignPrev(l2);
    l2->activation = &NetMath::sigmoid;
    l1->init(0);
    l2->init(1);
    l3->init(2);

    std::vector<double> emptyVec;

    l2->neurons[0]->sum = 0;
    l2->neurons[1]->sum = 1;
    l2->neurons[2]->sum = 0;
    l2->neurons[0]->dropped = false;
    l2->neurons[1]->dropped = false;
    l2->neurons[2]->dropped = false;
    l2->backward(emptyVec);

    REQUIRE( l2->neurons[0]->derivative == 0.25 );
    REQUIRE( l2->neurons[1]->derivative == 0.19661193324148185 );
    REQUIRE( l2->neurons[2]->derivative == 0.25 );

    delete l1;
    delete l2;
    delete l3;
}

TEST_CASE("Layer::backward - Sets the neurons' errors to their derivative multiplied by weighted errors in next layer, when no expected data is passed") {
    Layer* l1 = new Layer(0, 2);
    Layer* l2 = new Layer(0, 3);
    Layer* l3 = new Layer(0, 4);
    l2->assignPrev(l1);
    l2->assignNext(l3);
    l3->assignPrev(l2);
    l2->activation = &NetMath::sigmoid;
    l1->init(0);
    l2->init(1);
    l3->init(2);

    std::vector<double> emptyVec;

    l2->neurons[0]->sum = 0.5;
    l2->neurons[1]->sum = 0.5;
    l2->neurons[2]->sum = 0.5;
    l2->neurons[0]->dropped = false;
    l2->neurons[1]->dropped = false;
    l2->neurons[2]->dropped = false;

    for (int i=0; i<4; i++) {
        l3->neurons[i]->error = 0.5;
        l3->neurons[i]->weights = {1,1,1,1};
    }

    l2->backward(emptyVec);

    REQUIRE( l2->neurons[0]->error == 0.470007424403189 );
    REQUIRE( l2->neurons[1]->error == 0.470007424403189 );
    REQUIRE( l2->neurons[2]->error == 0.470007424403189 );

    delete l1;
    delete l2;
    delete l3;
}

TEST_CASE("Layer::backward - Increments each of its delta weights by its error * the respective weight's neuron's activation") {
    Layer* l1 = new Layer(0, 2);
    Layer* l2 = new Layer(0, 3);
    Layer* l3 = new Layer(0, 4);
    l2->assignPrev(l1);
    l2->assignNext(l3);
    l3->assignPrev(l2);
    l2->activation = &NetMath::sigmoid;
    l1->init(0);
    l2->init(1);
    l3->init(2);

    std::vector<double> expected = {1,2,3,4};

    Network::getInstance(0)->l2 = 0;

    l2->neurons[0]->activation = 0.5;
    l2->neurons[1]->activation = 0.5;
    l2->neurons[2]->activation = 0.5;
    l3->neurons[0]->dropped = false;
    l3->neurons[1]->dropped = false;
    l3->neurons[2]->dropped = false;
    l3->neurons[3]->dropped = false;

    for (int i=0; i<4; i++) {
        l3->neurons[i]->activation = 0.5;
    }

    l3->backward(expected);

    for (int n=0; n<4; n++) {
        REQUIRE( l3->neurons[n]->deltaWeights[0] == 0.25 + n * 0.5 );
        REQUIRE( l3->neurons[n]->deltaWeights[1] == 0.25 + n * 0.5 );
        REQUIRE( l3->neurons[n]->deltaWeights[2] == 0.25 + n * 0.5 );
    }

    delete l1;
    delete l2;
    delete l3;
}

TEST_CASE("Layer::backward - Sets the neurons' deltaBias to their errors") {
    Layer* l1 = new Layer(0, 2);
    Layer* l2 = new Layer(0, 3);
    l2->assignPrev(l1);
    l1->init(0);
    l2->init(1);

    std::vector<double> expected = {1,2,3};

    l2->neurons[0]->activation = 0;
    l2->neurons[1]->activation = 1;
    l2->neurons[2]->activation = 0;
    l2->neurons[0]->dropped = false;
    l2->neurons[1]->dropped = false;
    l2->neurons[2]->dropped = false;

    l2->backward(expected);

    REQUIRE( l2->neurons[0]->error == 1 );
    REQUIRE( l2->neurons[0]->deltaBias == 1 );
    REQUIRE( l2->neurons[1]->error == 1 );
    REQUIRE( l2->neurons[1]->deltaBias == 1 );
    REQUIRE( l2->neurons[2]->error == 3 );
    REQUIRE( l2->neurons[2]->deltaBias == 3 );

    delete l1;
    delete l2;
}

TEST_CASE("Layer::backward - Sets the neurons' error and deltaBias values to 0 when they are dropped") {
    Layer* l1 = new Layer(0, 2);
    Layer* l2 = new Layer(0, 3);
    l2->assignPrev(l1);
    l1->init(0);
    l2->init(1);

    std::vector<double> expected = {1,2,3};

    l2->neurons[0]->activation = 0;
    l2->neurons[1]->activation = 1;
    l2->neurons[2]->activation = 0;

    l2->neurons[0]->dropped = true;
    l2->neurons[1]->dropped = true;
    l2->neurons[2]->dropped = true;
    l2->neurons[0]->error = 123;
    l2->neurons[1]->error = 123;
    l2->neurons[2]->error = 123;
    l2->neurons[0]->deltaBias = 456;
    l2->neurons[2]->deltaBias = 456;
    l2->neurons[1]->deltaBias = 456;

    l2->backward(expected);

    REQUIRE( l2->neurons[0]->error == 0 );
    REQUIRE( l2->neurons[1]->error == 0 );
    REQUIRE( l2->neurons[2]->error == 0 );

    REQUIRE( l2->neurons[0]->deltaBias == 0 );
    REQUIRE( l2->neurons[1]->deltaBias == 0 );
    REQUIRE( l2->neurons[2]->deltaBias == 0 );

    delete l1;
    delete l2;
}

TEST_CASE("Layer::backward - Increments the deltaWeights by the orig value, multiplied by the l2 amount * existing deltaWeight value") {
    Layer* l1 = new Layer(0, 2);
    Layer* l2 = new Layer(0, 3);
    Layer* l3 = new Layer(0, 4);
    l2->assignPrev(l1);
    l2->assignNext(l3);
    l3->assignPrev(l2);
    l2->activation = &NetMath::sigmoid;
    l1->init(0);
    l2->init(1);
    l3->init(2);

    std::vector<double> expected = {0.3, 0.3, 0.3, 0.3};
    Network::getInstance(0)->l2 = 0.001;

    l2->neurons[0]->activation = 0.5;
    l2->neurons[1]->activation = 0.5;
    l2->neurons[2]->activation = 0.5;
    l3->neurons[0]->dropped = false;
    l3->neurons[1]->dropped = false;
    l3->neurons[2]->dropped = false;
    l3->neurons[3]->dropped = false;

    for (int i=0; i<4; i++) {
        l3->neurons[i]->activation = 0.25;
        l3->neurons[i]->deltaWeights[0] = 0.25;
        l3->neurons[i]->deltaWeights[1] = 0.25;
        l3->neurons[i]->deltaWeights[2] = 0.25;
    }

    l3->backward(expected);

    for (int n=0; n<4; n++) {
        REQUIRE( moreOrLessEqual(l3->neurons[n]->deltaWeights[0], 0.27500625, 6) );
        REQUIRE( moreOrLessEqual(l3->neurons[n]->deltaWeights[1], 0.27500625, 6) );
        REQUIRE( moreOrLessEqual(l3->neurons[n]->deltaWeights[2], 0.27500625, 6) );
    }

    delete l1;
    delete l2;
    delete l3;
}

TEST_CASE("Layer::backward - Increments the deltaWeights by the orig value, multiplied by the l1 amount * existing deltaWeight value") {
    Layer* l1 = new Layer(0, 2);
    Layer* l2 = new Layer(0, 3);
    Layer* l3 = new Layer(0, 4);
    l2->assignPrev(l1);
    l2->assignNext(l3);
    l3->assignPrev(l2);
    l2->activation = &NetMath::sigmoid;
    l1->init(0);
    l2->init(1);
    l3->init(2);

    std::vector<double> expected = {0.3, 0.3, 0.3, 0.3};
    Network::getInstance(0)->l1 = 0.005;

    l2->neurons[0]->activation = 0.5;
    l2->neurons[1]->activation = 0.5;
    l2->neurons[2]->activation = 0.5;
    l3->neurons[0]->dropped = false;
    l3->neurons[1]->dropped = false;
    l3->neurons[2]->dropped = false;
    l3->neurons[3]->dropped = false;

    for (int i=0; i<4; i++) {
        l3->neurons[i]->activation = 0.25;
        l3->neurons[i]->deltaWeights = {0.25, 0.25, 0.25, 0.25};
    }

    l3->backward(expected);

    for (int n=0; n<4; n++) {
        REQUIRE( moreOrLessEqual(l3->neurons[n]->deltaWeights[0], 0.275031, 4) );
        REQUIRE( moreOrLessEqual(l3->neurons[n]->deltaWeights[1], 0.275031, 4) );
        REQUIRE( moreOrLessEqual(l3->neurons[n]->deltaWeights[2], 0.275031, 4) );
    }

    delete l1;
    delete l2;
    delete l3;
}


TEST_CASE("Layer::applyDeltaWeights - Increments the weights by the delta weights") {
    Layer* l1 = new Layer(0, 2);
    Layer* l2 = new Layer(0, 3);
    l2->assignPrev(l1);
    l2->init(1);
    Network::getInstance(0)->updateFnIndex = 0;
    l2->netInstance = 0;

    for (int n=1; n<3; n++) {
        l2->neurons[n]->weights = {1,1,1};
        l2->neurons[n]->deltaWeights = {1,2,3};
    }

    l2->applyDeltaWeights();
    std::vector<double> expected = {2,3,4};

    for (int n=1; n<3; n++) {
        REQUIRE( l2->neurons[n]->weights == expected );
    }

    delete l1;
    delete l2;
}

TEST_CASE("Layer::applyDeltaWeights - Increments the bias by the deltaBias") {
    Network::getInstance(0)->learningRate = 1;
    Network::getInstance(0)->updateFnIndex = 0;
    Layer* l1 = new Layer(0, 2);
    Layer* l2 = new Layer(0, 3);
    l2->netInstance = 0;
    l2->assignPrev(l1);
    l2->init(1);

    for (int n=0; n<3; n++) {
        l2->neurons[n]->bias = n;
        l2->neurons[n]->deltaBias = n*2;
    }

    l2->applyDeltaWeights();

    REQUIRE( l2->neurons[0]->bias == 0 );
    REQUIRE( l2->neurons[1]->bias == 3 );
    REQUIRE( l2->neurons[2]->bias == 6 );

    delete l1;
    delete l2;
}

TEST_CASE("Layer::applyDeltaWeights - Increments the l2Error by the l2 formula applied to all weights") {
    Network* net = Network::getInstance(0);
    net->l2 = 0.001;
    net->updateFnIndex = 0;
    net->learningRate = 0.2;
    net->l2Error = 0;
    Layer* l1 = new Layer(0, 1);
    Layer* l2 = new Layer(0, 1);
    l2->netInstance = 0;
    l2->assignPrev(l1);
    l2->init(1);

    l2->neurons[0]->weights = {0.25, 0.25};
    l2->neurons[0]->deltaWeights = {0.5, 0.5};

    l2->applyDeltaWeights();

    REQUIRE( moreOrLessEqual( net->l2Error, 0.0000625 , 6) );

    delete l1;
    delete l2;
}

TEST_CASE("Layer::applyDeltaWeights - Increments the l1Error by the l1 formula applied to all weights") {
    Network* net = Network::getInstance(0);
    net->l1 = 0.005;
    net->updateFnIndex = 0;
    net->learningRate = 0.2;
    net->l1Error = 0;
    Layer* l1 = new Layer(0, 1);
    Layer* l2 = new Layer(0, 1);
    l2->netInstance = 0;
    l2->assignPrev(l1);
    l2->init(1);

    l2->neurons[0]->weights = {0.25, 0.25};
    l2->neurons[0]->deltaWeights = {0.5, 0.5};

    l2->applyDeltaWeights();

    REQUIRE( moreOrLessEqual( net->l1Error, 0.0025 , 6) );

    delete l1;
    delete l2;
}

TEST_CASE("Layer::resetDeltaWeights - Sets all deltaWeight values to 0") {
    Layer* l1 = new Layer(0, 2);
    Layer* l2 = new Layer(0, 3);
    l2->assignPrev(l1);
    l2->init(1);

    for (int n=1; n<3; n++) {
        l2->neurons[n]->deltaWeights = {1,2,3};
    }
    std::vector<double> expected = {0,0,0};

    l2->resetDeltaWeights();

    for (int n=1; n<3; n++) {
        REQUIRE(l2->neurons[n]->deltaWeights == expected);
    }

    delete l1;
    delete l2;
}



/* Neuron */
TEST_CASE("Neuron::init - Fills the deltaWeights vector with 0 values, matching weights size") {
    Neuron* testN = new Neuron();
    testN->weights = {1,2,3,4,5};
    testN->init(0);
    REQUIRE( testN->deltaWeights.size() == 5 );
    delete testN;
}

TEST_CASE("Neuron::init - Sets the neuron deltaBias to 0") {
    Neuron* testN = new Neuron();
    testN->init(0);
    REQUIRE( testN->deltaBias == 0 );
    delete testN;
}

TEST_CASE("Neuron::init - Sets the neuron biasGain to 1 if the net's updateFn is gain") {
    Network* net = Network::getInstance(0);
    Neuron* testN = new Neuron();
    net->updateFnIndex = 1;
    testN->init(0);
    REQUIRE( testN->biasGain == 1 );
    delete testN;
}

TEST_CASE("Neuron::init - Sets the neuron weightGain to a vector of 1s, with the same size as the weights vector when updateFn is gain") {
    Network* net = Network::getInstance(0);
    Neuron* testN = new Neuron();
    testN->weights = {1,2,3,4,5};
    net->updateFnIndex = 1;
    testN->init(0);
    std::vector<double> expected = {1,1,1,1,1};
    REQUIRE( testN->weightGain == expected );
    delete testN;
}

TEST_CASE("Neuron::init - Does not set the biasGain or weightGain to anything if updateFn is not gain") {
    Network* net = Network::getInstance(0);
    Neuron* testN = new Neuron();
    net->updateFnIndex = 2;
    testN->init(0);
    // REQUIRE( testN->biasGain == 0 );
    REQUIRE( testN->weightGain.size() == 0 );
    delete testN;
}

TEST_CASE("Neuron::init - Sets the neuron biasCache to 0 if the updateFn is adagrad") {
    Network* net = Network::getInstance(0);
    Neuron* testN = new Neuron();
    net->updateFnIndex = 2;
    testN->biasCache = 1;
    testN->init(0);
    REQUIRE( testN->biasCache == 0 );
    delete testN;
}

TEST_CASE("Neuron::init - Sets the neuron weightsCache to a vector of zeroes with the same size as the weights when updateFn is adagrad") {
    Network* net = Network::getInstance(0);
    Neuron* testN = new Neuron();
    testN->weights = {1,2,3,4,5};
    net->updateFnIndex = 2;
    testN->init(0);
    std::vector<double> expected = {0,0,0,0,0};
    REQUIRE( testN->weightsCache == expected );
    delete testN;
}

TEST_CASE("Neuron::init - Does not set the biasCache or weightsCache to anything if updateFn is not adagrad") {
    Network* net = Network::getInstance(0);
    Neuron* testN = new Neuron();
    net->updateFnIndex = 1;
    testN->biasCache = 12234;
    testN->init(0);
    REQUIRE( testN->biasCache == 12234 );
    REQUIRE( testN->weightsCache.size() == 0 );
    delete testN;
}

TEST_CASE("Neuron::init - Sets the neuron biasCache to 0 if the updateFn is rmsprop") {
    Network* net = Network::getInstance(0);
    Neuron* testN = new Neuron();
    net->updateFnIndex = 3;
    testN->biasCache = 1;
    testN->init(0);
    REQUIRE( testN->biasCache == 0 );
    delete testN;
}

TEST_CASE("Neuron::init - Sets the neuron weightsCache to a vector of zeroes with the same size as the weights when updateFn is rmsprop") {
    Network* net = Network::getInstance(0);
    Neuron* testN = new Neuron();
    testN->weights = {1,2,3,4,5};
    net->updateFnIndex = 3;
    testN->init(0);
    std::vector<double> expected = {0,0,0,0,0};
    REQUIRE( testN->weightsCache == expected );
    delete testN;
}

TEST_CASE("Neuron::init - Sets the neuron m and neuron v to 0 if the updateFn is adam") {
    Network* net = Network::getInstance(0);
    Neuron* testN = new Neuron();
    net->updateFnIndex = 4;
    testN->m = 1;
    testN->v = 1;
    testN->init(0);
    REQUIRE( testN->m == 0 );
    REQUIRE( testN->v == 0 );
    delete testN;
}

TEST_CASE("Neuron::init - Does not set the neuron m and neuron v to 0 if the updateFn is not adam") {
    Network* net = Network::getInstance(0);
    Neuron* testN = new Neuron();
    net->updateFnIndex = 3;
    testN->m = 1;
    testN->v = 1;
    testN->init(0);
    REQUIRE( testN->m == 1 );
    REQUIRE( testN->v == 1 );
    delete testN;
}

TEST_CASE("Neuron::init - Sets the neuron biasCache and adadeltaBiasCache to 0 if the updateFn is adadelta") {
    Network* net = Network::getInstance(0);
    Neuron* testN = new Neuron();
    net->updateFnIndex = 5;
    testN->biasCache = 1;
    testN->adadeltaBiasCache = 1;
    testN->init(0);
    REQUIRE( testN->biasCache == 0 );
    REQUIRE( testN->adadeltaBiasCache == 0 );
    delete testN;
}

TEST_CASE("Neuron::init - Sets the neuron weightsCache and adadeltaCache to a vector of zeroes with the same size as the weights when updateFn is adadelta") {
    Network* net = Network::getInstance(0);
    Neuron* testN = new Neuron();
    testN->weights = {1,2,3,4,5};
    net->updateFnIndex = 5;
    testN->init(0);
    std::vector<double> expected = {0,0,0,0,0};
    REQUIRE( testN->weightsCache == expected );
    REQUIRE( testN->adadeltaCache == expected );
    delete testN;
}

TEST_CASE("Neuron::init - Does not set the biasCache or weightsCache to anything if updateFn is not adadelta") {
    Network* net = Network::getInstance(0);
    Neuron* testN = new Neuron();
    net->updateFnIndex = 1;
    testN->biasCache = 12234;
    testN->adadeltaBiasCache = 12234;
    testN->init(0);
    REQUIRE( testN->biasCache == 12234 );
    REQUIRE( testN->adadeltaBiasCache == 12234 );
    REQUIRE( testN->weightsCache.size() == 0 );
    REQUIRE( testN->adadeltaCache.size() == 0 );
    delete testN;
}

TEST_CASE("Neuron::init - Sets the network lreluSlope to the neuron, if the activation function is lrelu") {
    Network* net = Network::getInstance(0);
    net->activation = &NetMath::lrelu;
    net->lreluSlope = 0.1;
    Neuron* testN = new Neuron();
    testN->init(0);
    REQUIRE( moreOrLessEqual(testN->lreluSlope, 0.1, 6) );
    delete testN;
}

TEST_CASE("Neuron::init - Sets the neuron rreluSlope to a number if the activation is rrelu") {
    Network* net = Network::getInstance(0);
    net->activation = &NetMath::rrelu;
    Neuron* testN = new Neuron();
    testN->rreluSlope = 0.1;
    testN->init(0);
    REQUIRE( testN->rreluSlope != 0 );
    REQUIRE( testN->rreluSlope != 0.1 );
    REQUIRE( testN->rreluSlope >= -0.1);
    REQUIRE( testN->rreluSlope <= 0.1);
    delete testN;
}

TEST_CASE("Neuron::init - Sets the network eluAlpha to the neuron, if the activation function is elu") {
    Network* net = Network::getInstance(0);
    net->activation = &NetMath::elu;
    net->eluAlpha = 0.1;
    Neuron* testN = new Neuron();
    testN->init(0);
    REQUIRE( moreOrLessEqual(testN->eluAlpha, 0.1, 6) );
    delete testN;
}

/* NetMath */
/*
    NOTE: all expected values were copied from the js unit tests, where they were calculated by hand
*/
TEST_CASE("NetMath::sigmoid") {
    Neuron* testN = new Neuron();
    REQUIRE( NetMath::sigmoid(1.681241237, false, testN) == 0.8430688214048092 );
    REQUIRE( NetMath::sigmoid(0.8430688214048092, true, testN) == 0.21035474941074114 );
    delete testN;
}

TEST_CASE("NetMath::tanh") {
    Neuron* testN = new Neuron();
    REQUIRE( doublesAreEqual(NetMath::tanh(1, false, testN), 0.7615941559557649) );
    REQUIRE( doublesAreEqual(NetMath::tanh(0.5, false, testN), 0.46211715726000974));
    REQUIRE( doublesAreEqual(NetMath::tanh(0.5, true, testN), 0.7864477329659275) );
    REQUIRE( doublesAreEqual(NetMath::tanh(1.5, true, testN), 0.18070663892364855) );
    REQUIRE( NetMath::tanh(900, true, testN)!=NAN );
    delete testN;
}

TEST_CASE("NetMath::lecuntanh") {
    Neuron* testN = new Neuron();
    REQUIRE( NetMath::lecuntanh(2.0, false, testN) == 1.4929388053842507 );
    REQUIRE( NetMath::lecuntanh(-2.0, false, testN) == -1.4929388053842507 );
    REQUIRE( NetMath::lecuntanh(2.0, true, testN) == 0.2802507761872869 );
    REQUIRE( NetMath::lecuntanh(-2.0, true, testN) == 0.2802507761872869 );
    delete testN;
}

TEST_CASE("NetMath::relu") {
    Neuron* testN = new Neuron();
    REQUIRE( NetMath::relu(2, false, testN) == 2 );
    REQUIRE( NetMath::relu(-2, false, testN) == 0 );
    REQUIRE( NetMath::relu(2, true, testN) == 1 );
    REQUIRE( NetMath::relu(-2, true, testN) == 0 );
    delete testN;
}

TEST_CASE("NetMath::lrelu") {
    Neuron* testN = new Neuron();
    testN->lreluSlope = -0.0005;
    REQUIRE( NetMath::lrelu(2, false, testN) == 2 );
    REQUIRE( NetMath::lrelu(-2, false, testN) == -0.001 );
    REQUIRE( NetMath::lrelu(2, true, testN) == 1 );
    REQUIRE( NetMath::lrelu(-2, true, testN) == -0.0005 );
    delete testN;
}

TEST_CASE("NetMath::rrelu") {
    Neuron* testN = new Neuron();
    testN->rreluSlope = 0.0005;
    REQUIRE( NetMath::rrelu(2, false, testN) == 2 );
    REQUIRE( NetMath::rrelu(-2, false, testN) == 0.0005 );
    REQUIRE( NetMath::rrelu(2, true, testN) == 1 );
    REQUIRE( NetMath::rrelu(-2, true, testN) == 0.0005 );
    delete testN;
}

TEST_CASE("NetMath::elu") {
    Neuron* testN = new Neuron();
    testN->eluAlpha = 1;
    REQUIRE( NetMath::elu(2, false, testN) == 2 );
    REQUIRE( NetMath::elu(-0.25, false, testN) == -0.22119921692859512 );
    REQUIRE( NetMath::elu(2, true, testN) == 1 );
    REQUIRE( NetMath::elu(-0.5, true, testN) == 0.6065306597126334 );
    delete testN;
}

TEST_CASE("NetMath::meansquarederror") {
    std::vector<double> values1 = {13,17,18,20,24};
    std::vector<double> values2 = {12,15,20,22,24};
    REQUIRE(NetMath::meansquarederror(values1, values2)==(double)2.6 );
}

TEST_CASE("NetMath::crossentropy") {
    std::vector<double> values1 = {1, 0, 0.3};
    std::vector<double> values2 = {0, 1, 0.8};
    REQUIRE( NetMath::crossentropy(values1, values2) == (double)70.16654147569186 );
}

TEST_CASE("NetMath::vanillaupdatefn") {
    Network::getInstance(0)->learningRate = 0.5;
    REQUIRE( NetMath::vanillaupdatefn(0, 10, 10)==15 );
    REQUIRE( NetMath::vanillaupdatefn(0, 10, 20)==20 );
    REQUIRE( NetMath::vanillaupdatefn(0, 10, -30)==-5 );
}

TEST_CASE("NetMath::gain - Doubles a value when the gain is 2 and learningRate 1") {
    Neuron* testN = new Neuron();
    testN->init(0);
    Network::getInstance(0)->learningRate = 1;
    testN->biasGain = 2;
    REQUIRE( NetMath::gain(0, (double)10, (double)5, testN, -1) == 20 );
    delete testN;
}

TEST_CASE("NetMath::gain - Halves a value when the gain is -5 and learningRate 0.1") {
    Neuron* testN = new Neuron();
    testN->init(0);
    Network::getInstance(0)->learningRate = 0.1;
    testN->biasGain = -5;
    double res = NetMath::gain(0, (double)5, (double)5, testN, -1);
    REQUIRE( moreOrLessEqual(res, (double)2.5, 2) );
    delete testN;
}

TEST_CASE("NetMath::gain - Increments a neuron's biasGain by 0.05 when the bias value doesn't change sign") {
    Neuron* testN = new Neuron();
    testN->init(0);
    Network::getInstance(0)->learningRate = 1;
    testN->bias = 0.1;
    testN->biasGain = 1;
    NetMath::gain(0, (double)0.1, (double)1, testN, -1);
    REQUIRE( testN->biasGain == 1.05 );
    delete testN;
}

TEST_CASE("NetMath::gain - Does not increase the gain to more than 5") {
    Neuron* testN = new Neuron();
    testN->init(0);
    Network::getInstance(0)->learningRate = 1;
    testN->bias = 0.1;
    testN->biasGain = 4.99;
    NetMath::gain(0, (double)0.1, (double)1, testN, -1);
    REQUIRE( testN->biasGain == 5 );
    delete testN;
}

TEST_CASE("NetMath::gain - Multiplies a neuron's bias gain by 0.95 when the value changes sign") {
    Neuron* testN = new Neuron();
    testN->init(0);
    Network::getInstance(0)->learningRate = -10;
    testN->bias = 0.1;
    testN->biasGain = 1;
    NetMath::gain(0, (double)0.1, (double)1, testN, -1);
    REQUIRE( testN->biasGain == 0.95 );
    delete testN;
}

TEST_CASE("NetMath::gain - Does not reduce the bias gain to less than 0.5") {
    Neuron* testN = new Neuron();
    testN->init(0);
    Network::getInstance(0)->learningRate = -10;
    testN->bias = 0.1;
    testN->biasGain = 0.51;
    NetMath::gain(0, (double)0.1, (double)1, testN, -1);
    REQUIRE( testN->biasGain == 0.5 );
    delete testN;
}

TEST_CASE("NetMath::gain - Increases weight gain the same way as the bias gain") {
    Neuron* testN = new Neuron();
    Network::getInstance(0)->learningRate = 1;
    testN->weights = {0.1, 0.1};
    testN->weightGain = {1, 4.99};
    NetMath::gain(0, (double)0.1, (double)1, testN, 0);
    NetMath::gain(0, (double)0.1, (double)1, testN, 1);
    REQUIRE( testN->weightGain[0] == 1.05 );
    REQUIRE( testN->weightGain[1] == 5 );
    delete testN;
}

TEST_CASE("NetMath::gain - Decreases weight gain the same way as the bias gain") {
    Neuron* testN = new Neuron();
    Network::getInstance(0)->learningRate = -10;
    testN->weights = {0.1, 0.1};
    testN->weightGain = {1, 0.51};
    NetMath::gain(0, (double)0.1, (double)1, testN, 0);
    NetMath::gain(0, (double)0.1, (double)1, testN, 1);
    REQUIRE( testN->weightGain[0] == 0.95 );
    REQUIRE( testN->weightGain[1] == 0.5 );
    delete testN;
}

TEST_CASE("NetMath::adagrad - Increments the neuron's biasCache by the square of its deltaBias") {
    Neuron* testN = new Neuron();
    Network::getInstance(0)->learningRate = 2;
    testN->biasCache = 0;
    NetMath::adagrad(0, (double)1, (double)3, testN, -1);
    REQUIRE( testN->biasCache == 9 );
    delete testN;
}

TEST_CASE("NetMath::adagrad - Returns a new value matching the formula for adagrad") {
    Neuron* testN = new Neuron();
    Network::getInstance(0)->learningRate = 0.5;
    testN->biasCache = 0;
    double result = NetMath::adagrad(0, (double)1, (double)3, testN, -1);
    REQUIRE( moreOrLessEqual(result, 1.5, 2) );
    delete testN;
}

TEST_CASE("NetMath::adagrad - Increments the neuron's weightsCache with the same way as the biasCache") {
    Neuron* testN = new Neuron();
    Network::getInstance(0)->learningRate = 2;
    testN->weightsCache = {0, 1, 2};
    double result1 = NetMath::adagrad(0, (double)1, (double)3, testN, 0);
    double result2 = NetMath::adagrad(0, (double)1, (double)4, testN, 1);
    double result3 = NetMath::adagrad(0, (double)1, (double)2, testN, 2);
    REQUIRE( testN->weightsCache[0] == 9 );
    REQUIRE( testN->weightsCache[1] == 17 );
    REQUIRE( testN->weightsCache[2] == 6 );
    REQUIRE( moreOrLessEqual(result1, 3.0, 2) );
    REQUIRE( moreOrLessEqual(result2, 2.9, 1) );
    REQUIRE( moreOrLessEqual(result3, 2.6, 1) );
    delete testN;
}

TEST_CASE("NetMath::rmsprop - Sets the cache value to the correct value, following the rmsprop formula") {
    Neuron* testN = new Neuron();
    Network::getInstance(0)->learningRate = 2;
    Network::getInstance(0)->rmsDecay = 0.99;
    testN->biasCache = 10;
    NetMath::rmsprop(0, (double)1, (double)3, testN, -1);
    REQUIRE( moreOrLessEqual(testN->biasCache, 9.99, 3) );
    delete testN;
}

TEST_CASE("NetMath::rmsprop - Returns a new value matching the formula for rmsprop, using this new cache value") {
    Neuron* testN = new Neuron();
    Network::getInstance(0)->learningRate = 0.5;
    Network::getInstance(0)->rmsDecay = 0.99;
    testN->biasCache = 10;
    double result = NetMath::rmsprop(0, (double)1, (double)3, testN, -1);
    REQUIRE( moreOrLessEqual(result, 1.47, 2) );
    delete testN;
}

TEST_CASE("NetMath::rmsprop - Updates the weightsCache the same way as the biasCache") {
    Neuron* testN = new Neuron();
    Network::getInstance(0)->learningRate = 0.5;
    Network::getInstance(0)->rmsDecay = 0.99;
    testN->weightsCache = {0, 1, 2};
    double result1 = NetMath::rmsprop(0, (double)1, (double)3, testN, 0);
    double result2 = NetMath::rmsprop(0, (double)1, (double)4, testN, 1);
    double result3 = NetMath::rmsprop(0, (double)1, (double)2, testN, 2);
    REQUIRE( moreOrLessEqual(testN->weightsCache[0], 0.09, 2) );
    REQUIRE( moreOrLessEqual(testN->weightsCache[1], 1.15, 2) );
    REQUIRE( moreOrLessEqual(testN->weightsCache[2], 2.02, 2) );
    REQUIRE( moreOrLessEqual(result1, 6.0, 2) );
    REQUIRE( moreOrLessEqual(result2, 2.9, 1) );
    REQUIRE( moreOrLessEqual(result3, 1.7, 1) );
    delete testN;
}

TEST_CASE("NetMath::adam - It sets the neuron.m to the correct value, following the formula") {
    Neuron* testN = new Neuron();
    Network::getInstance(0)->learningRate = 0.01;
    testN->m = 0.1;
    NetMath::adam(0, (double)1, (double)0.2, testN, -1);
    REQUIRE( doublesAreEqual(testN->m, 0.11) );
    delete testN;
}

TEST_CASE("NetMath::adam - It sets the neuron.v to the correct value, following the formula") {
    Neuron* testN = new Neuron();
    Network::getInstance(0)->learningRate = 0.01;
    testN->v = 0.1;
    NetMath::adam(0, (double)1, (double)0.2, testN, -1);
    REQUIRE( moreOrLessEqual(testN->v, 0.09994, 3) );
    delete testN;
}

TEST_CASE("NetMath::adam - Calculates a value correctly, following the formula") {
    Neuron* testN = new Neuron();
    Network::getInstance(0)->learningRate = 0.01;
    Network::getInstance(0)->iterations = 2;
    testN->m = 0.121;
    testN->v = 0.045;
    double result = NetMath::adam(0, (double)-0.3, (double)0.02, testN, -1);
    REQUIRE( moreOrLessEqual(result, -0.298943, 5) );
    delete testN;
}

TEST_CASE("NetMath::adadelta - Sets the neuron.biasCache to the correct value, following the adadelta formula") {
    Network::deleteNetwork();
    Network::newNetwork();
    Network::getInstance(0)->weightsConfig["limit"] = 0.1;
    Network::getInstance(0)->weightInitFn = &NetMath::uniform;
    Neuron* testN = new Neuron();
    Network::getInstance(0)->rho = (double)0.95;
    testN->biasCache = (double)0.5;
    NetMath::adadelta(0, (double)0.5, (double)0.2, testN, -1);
    REQUIRE( moreOrLessEqual(testN->biasCache, 0.477, 3) );
    delete testN;
}

TEST_CASE("NetMath::adadelta - Sets the weightsCache to the correct value, following the adadelta formula, same as biasCache") {
    Neuron* testN = new Neuron();
    Network::getInstance(0)->rho = 0.95;
    testN->weightsCache = {0.5, 0.75};
    testN->adadeltaCache = {0, 0};
    NetMath::adadelta(0, (double)0.5, (double)0.2, testN, 0);
    NetMath::adadelta(0, (double)0.5, (double)0.2, testN, 1);
    REQUIRE( moreOrLessEqual(testN->weightsCache[0], 0.477, 3) );
    REQUIRE( moreOrLessEqual(testN->weightsCache[1], 0.7145, 4) );
    delete testN;
}

TEST_CASE("NetMath::adadelta - Creates a value for the bias correctly, following the formula") {
    Neuron* testN = new Neuron();
    Network::getInstance(0)->rho = 0.95;
    testN->biasCache = 0.5;
    testN->adadeltaBiasCache = 0.25;
    double val = NetMath::adadelta(0, (double)0.5, (double)0.2, testN, -1);
    REQUIRE( moreOrLessEqual(val, 0.64479, 5) );
    delete testN;
}

TEST_CASE("NetMath::adadelta - Creates a value for the weight correctly, the same was as the bias") {
    Neuron* testN = new Neuron();
    Network::getInstance(0)->rho = 0.95;
    testN->weightsCache = {0.5, 0.75};
    testN->adadeltaCache = {0.1, 0.2};
    double val1 = NetMath::adadelta(0, (double)0.5, (double)0.2, testN, 0);
    double val2 = NetMath::adadelta(0, (double)0.5, (double)0.2, testN, 1);
    REQUIRE( moreOrLessEqual(val1, 0.59157, 3) );
    REQUIRE( moreOrLessEqual(val2, 0.60581, 3) );
    delete testN;
}

TEST_CASE("NetMath::adadelta - Updates the neuron.adadeltaBiasCache with the correct value, following the formula") {
    Neuron* testN = new Neuron();
    Network::getInstance(0)->rho = (double)0.95;
    testN->biasCache = (double)0.5;
    testN->adadeltaBiasCache = (double)0.25;
    NetMath::adadelta(0, (double)0.5, (double)0.2, testN, -1);
    REQUIRE( moreOrLessEqual(testN->adadeltaBiasCache, 0.2395, 2) );
    delete testN;
}

TEST_CASE("NetMath::adadelta - Updates the neuron.adadeltaCache with the correct value, following the formula, same as adadeltaBiasCache") {
    Neuron* testN = new Neuron();
    Network::getInstance(0)->rho = 0.95;
    testN->weightsCache = {0.5, 0.75};
    testN->adadeltaCache = {0.1, 0.2};
    NetMath::adadelta(0, (double)0.5, (double)0.2, testN, 0);
    NetMath::adadelta(0, (double)0.5, (double)0.2, testN, 1);
    REQUIRE( moreOrLessEqual(testN->adadeltaCache[0], 0.097, 1) );
    REQUIRE( moreOrLessEqual(testN->adadeltaCache[1], 0.192, 1) );
    delete testN;
}

TEST_CASE("NetMath::sech - Calculates values correctly") {
    REQUIRE( doublesAreEqual(NetMath::sech(1), 0.6480542736638853) );
    REQUIRE( doublesAreEqual(NetMath::sech(-0.5), 0.886818883970074) );
}


TEST_CASE("NetMath::maxNorm - Sets the maxNormTotal to 0") {
    Network::getInstance(0)->maxNormTotal = 1;
    NetMath::maxNorm(0);
    REQUIRE( Network::getInstance(0)->maxNormTotal == 0 );
}


TEST_CASE("NetMath::maxNorm - Scales weights if their L2 exceeds the configured max norm threshold") {
    Network* net = Network::getInstance(0);
    net->layers.clear();
    net->maxNorm = 1;

    Layer* l1 = new Layer(0, 1);
    Layer* l2 = new Layer(0, 2);
    l2->assignPrev(l1);
    l1->init(0);
    l2->init(1);
    net->layers.push_back(l1);
    net->layers.push_back(l2);

    l2->neurons[0]->weights = {2, 2};
    net->maxNormTotal = 2.8284271247461903;

    NetMath::maxNorm(0);

    REQUIRE( l2->neurons[0]->weights[0] == 0.7071067811865475 );
    REQUIRE( l2->neurons[0]->weights[1] == 0.7071067811865475 );
    delete l1;
    delete l2;
}

TEST_CASE("NetMath::maxNorm - Does not scale weights if their L2 doesn't exceed the configured max norm threshold") {
    Network* net = Network::getInstance(0);
    net->layers.clear();
    net->maxNorm = 1000;

    Layer* l1 = new Layer(0, 1);
    Layer* l2 = new Layer(0, 2);
    l2->assignPrev(l1);
    l1->init(0);
    l2->init(1);
    net->layers.push_back(l1);
    net->layers.push_back(l2);

    l2->neurons[0]->weights = {2, 2};
    net->maxNormTotal = 2.8284271247461903;

    NetMath::maxNorm(0);

    REQUIRE( l2->neurons[0]->weights[0] == 2 );
    REQUIRE( l2->neurons[0]->weights[1] == 2 );
    delete l1;
    delete l2;
}

TEST_CASE("NetMath::uniform - Returns the same number of values as the size value given") {

    std::vector<double> values = NetMath::uniform(0, 0, 10);
    REQUIRE( values.size() == 10 );
}

TEST_CASE("NetMath::uniform - Weights are all between -0.1 and +0.1 when the limit is 0.1") {
    Network::getInstance(0)->weightsConfig["limit"] = 0.1;

    bool ok = true;
    std::vector<double> values = NetMath::uniform(0, 0, 100);

    for (int i=0; i<100; i++) {
        if (values[i] > 0.1 || values[i]<=-0.1) {
            ok = false;
        }
    }

    REQUIRE( ok );
}

TEST_CASE("NetMath::uniform - There are some weights bigger than |0.1| when the limit is 1000") {
    Network::getInstance(0)->weightsConfig["limit"] = 1000;

    bool ok = false;
    std::vector<double> values = NetMath::uniform(0, 0, 1000);

    for (int i=0; i<1000; i++) {
        if (values[i] > 0.1 || values[i]<=-0.1) {
            ok = true;
        }
    }

    REQUIRE( ok );
}

TEST_CASE("NetMath::gaussian - Returns the same number of values as the size value given") {
    std::vector<double> values = NetMath::gaussian(0, 0, 10);
    REQUIRE( values.size() == 10 );
}

TEST_CASE("NetMath:gaussian - The standard deviation of the weights is roughly 1 when set to 1") {
    Network::getInstance(0)->weightsConfig["stdDeviation"] = 1;
    std::vector<double> values = NetMath::gaussian(0, 0, 1000);
    double std = standardDeviation(values);
    REQUIRE( std<= 1.15 );
    REQUIRE( std>= 0.85 );
}

TEST_CASE("NetMath::gaussian - The standard deviation of the weights is roughly 5 when set to 5") {
    Network::getInstance(0)->weightsConfig["stdDeviation"] = 5;
    std::vector<double> values = NetMath::gaussian(0, 0, 1000);
    double std = standardDeviation(values);
    REQUIRE( std<= 1.15 * 5 );
    REQUIRE( std>= 0.85 * 5 );
}

TEST_CASE("NetMath::gaussian - The mean of the weights is roughly 0 when set to 0") {
    Network::getInstance(0)->weightsConfig["mean"] = 0;
    Network::getInstance(0)->weightsConfig["stdDeviation"] = 1;
    std::vector<double> values = NetMath::gaussian(0, 0, 1000);

    double total = 0;

    for (int v=0; v<1000; v++) {
        total += values[v];
    }

    total /= 1000;

    REQUIRE( total <= 0.1 );
    REQUIRE( total >= -0.1 );
}

TEST_CASE("NetMath::gaussian - The mean of the weights is roughly 10 when set to 10") {
    Network::getInstance(0)->weightsConfig["mean"] = 10;
    std::vector<double> values = NetMath::gaussian(0, 0, 1000);

    double total = 0;

    for (int v=0; v<1000; v++) {
        total += values[v];
    }

    total /= 1000;

    REQUIRE( total <= 10.1 );
    REQUIRE( total >= 9.85 );
}

TEST_CASE("NetMath::lecununiform - Returns the same number of values as the size value given") {
    std::vector<double> values = NetMath::lecununiform(0, 0, 10);
    REQUIRE( values.size() == 10 );
}

TEST_CASE("NetMath::lecununiform - Weights are all between -0.5 and +0.5 when fanIn is 12") {
    Network* net = Network::getInstance(0);
    net->layers.clear();
    Layer* l1 = new Layer(0, 1);
    l1->fanIn = 12;
    net->layers.push_back(l1);
    std::vector<double> values = NetMath::lecununiform(0, 0, 1000);

    bool ok = true;

    for (int i=0; i<1000; i++) {
        if (values[i] > 0.5 || values[i] < -0.5 ) {
            ok = false;
        }
    }

    REQUIRE( ok );
}

TEST_CASE("NetMath::lecununiform - Some weights are at values bigger than |0.5| when fanIn is smaller (8)") {
    Network* net = Network::getInstance(0);
    net->layers.clear();
    Layer* l1 = new Layer(0, 1);
    l1->fanIn = 8;
    net->layers.push_back(l1);
    std::vector<double> values = NetMath::lecununiform(0, 0, 1000);

    bool ok = false;

    for (int i=0; i<1000; i++) {
        if (values[i] > 0.5 || values[i] < -0.5 ) {
            ok = true;
        }
    }

    REQUIRE( ok );
}