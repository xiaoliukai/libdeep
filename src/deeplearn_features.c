/*
 libdeep - a library for deep learning
 Copyright (C) 2013-2015  Bob Mottram <bob@robotics.uk.to>

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:
 1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
 3. Neither the name of the University nor the names of its contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.
 .
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE HOLDERS OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "deeplearn_features.h"


/**
* @brief Learn a feature set between an input image and a neuron layer
* @param samples_across The number of units across in the second layer
* @param samples_down The number of units down in the second layer
* @param patch_radius The radius of the patch within the image
* @param image_width Width of the image
* @param image_height Height of the image
* @param image_depth Depth of the image (mono=1, RGB=3)
* @param img Image buffer
* @param layer0_units Number of units in the neuron layer
* @param feature_autocoder An autocoder used for feature learning
* @param BPerror Returned total learning error
* @returns zero on success
*/
int features_learn_from_image(int samples_across,
                              int samples_down,
                              int patch_radius,
                              int image_width,
                              int image_height,
                              int image_depth,
                              unsigned char * img,
                              int layer0_units,
                              bp * feature_autocoder,
                              float * BPerror)
{
    int no_of_learned_features = feature_autocoder->NoOfHiddens;
    *BPerror = 0;
    
    if (samples_across * samples_down * no_of_learned_features !=
        layer0_units) {
        /* across*down doesn't equal the second layer units */
        return -1;
    }

    if (feature_autocoder->NoOfInputs !=
        patch_radius*patch_radius*4*image_depth) {
        /* the patch size doesn't match the feature
           learner inputs */
        return -2;
    }

    if (feature_autocoder->NoOfInputs !=
        feature_autocoder->NoOfOutputs) {
        /* same number of inputs and outputs */
        return -3;
    }

    for (int fy = 0; fy < samples_down; fy++) {
        int cy = fy * image_height / samples_down;
        int ty = cy - patch_radius;
        int by = cy + patch_radius;
        if (ty < 0) ty = 0;
        if (by >= image_height) by = image_height-1;
        for (int fx = 0; fx < samples_across; fx++) {
            int cx = fx * image_width / samples_across;
            int tx = cx - patch_radius;
            int bx = cx + patch_radius;
            if (tx < 0) tx = 0;
            if (bx >= image_width) bx = image_width-1;

            /* scan the patch */
            int index_feature_input = 0;
            for (int y = ty; y < by; y++) {
                for (int x = tx; x < bx;
                     x++, index_feature_input++) {
                    if (index_feature_input >=
                        feature_autocoder->NoOfInputs) {
                        printf("Inputs out of range %d/%d\n",
                               index_feature_input,
                               feature_autocoder->NoOfInputs);
                    }
                    int index_image =
                        ((y*image_width) + x) *
                        image_depth;
                    /* convert from 8 bit to a neuron value */
                    float v =
                        0.25f + (img[index_image]/(2*255.0f));
                    bp_set_input(feature_autocoder,
                                 index_feature_input, v);
                    bp_set_output(feature_autocoder,
                                  index_feature_input, v);
                }
            }
            bp_update(feature_autocoder, 0);
            *BPerror = *BPerror + feature_autocoder->BPerror;
        }
    }
    return 0;
}

/**
* @brief Learn a feature set between an array of floats and a neuron layer
*        Inputs are expected to have values in the range 0.25->0.75
* @param samples_across The number of units across in the second layer
* @param samples_down The number of units down in the second layer
* @param patch_radius The radius of the patch within the inputs
* @param inputs_width Width of the inputs array
* @param inputs_height Height of the inputs array
* @param inputs_depth Depth of the inputs array
* @param img Inputs buffer
* @param layer0_units Number of units in the neuron layer
* @param feature_autocoder An autocoder used for feature learning
* @returns zero on success
*/
int features_learn_from_floats(int samples_across,
                               int samples_down,
                               int patch_radius,
                               int inputs_width,
                               int inputs_height,
                               int inputs_depth,
                               float * img,
                               int layer0_units,
                               bp * feature_autocoder)
{
    int no_of_learned_features = feature_autocoder->NoOfHiddens;

    if (samples_across * samples_down * no_of_learned_features !=
        layer0_units) {
        /* across*down doesn't equal the second layer units */
        return -1;
    }

    if (feature_autocoder->NoOfInputs !=
        patch_radius*patch_radius*4*inputs_depth) {
        /* the patch size doesn't match the feature
           learner inputs */
        return -2;
    }

    if (feature_autocoder->NoOfInputs !=
        feature_autocoder->NoOfOutputs) {
        /* same number of inputs and outputs */
        return -3;
    }

    for (int fy = 0; fy < samples_down; fy++) {
        int cy = fy * inputs_height / samples_down;
        int ty = cy - patch_radius;
        int by = cy + patch_radius;
        if (ty < 0) ty = 0;
        if (by >= inputs_height) by = inputs_height-1;
        for (int fx = 0; fx < samples_across; fx++) {
            int cx = fx * inputs_width / samples_across;
            int tx = cx - patch_radius;
            int bx = cx + patch_radius;
            if (tx < 0) tx = 0;
            if (bx >= inputs_width) bx = inputs_width-1;

            /* scan the patch */
            int index_feature_input = 0;
            for (int y = ty; y < by; y++) {
                for (int x = tx; x < bx;
                     x++, index_feature_input++) {
                    int index_inputs =
                        ((y*inputs_width) + x) *
                        inputs_depth;
                    /* convert from 8 bit to a neuron value */
                    bp_set_input(feature_autocoder,
                                 index_feature_input,
                                 img[index_inputs]);
                    bp_set_output(feature_autocoder,
                                  index_feature_input,
                                  img[index_inputs]);
                }
            }
            bp_update(feature_autocoder, 0);            
        }
    }
    return 0;
}

/**
* @brief Convolve an image with learned features and output
*        the results to the input layer of a neural net
* @param samples_across The number of units across in the input layer (sampling grid resolution)
* @param samples_down The number of units down in the input layer (sampling grid resolution)
* @param patch_radius The radius of the patch within the image
* @param image_width Width of the image
* @param image_height Height of the image
* @param image_depth Depth of the image (mono=1, RGB=3)
* @param img Image buffer
* @param layer0 Neural net
* @param feature_autocoder An autocoder containing learned features
* @returns zero on success
*/
int features_convolve_image_to_neurons(int samples_across,
                                       int samples_down,
                                       int patch_radius,
                                       int image_width,
                                       int image_height,
                                       int image_depth,
                                       unsigned char * img,
                                       bp * net,
                                       bp * feature_autocoder)
{
    int no_of_learned_features = feature_autocoder->NoOfHiddens;
    
    if (samples_across * samples_down * no_of_learned_features !=
        net->NoOfInputs) {
        /* across*down doesn't equal the second layer units */
        return -1;
    }

    if (feature_autocoder->NoOfInputs !=
        patch_radius*patch_radius*4*image_depth) {
        /* the patch size doesn't match the feature
           learner inputs */
        return -2;
    }

    if (feature_autocoder->NoOfInputs !=
        feature_autocoder->NoOfOutputs) {
        /* same number of inputs and outputs */
        return -3;
    }

    for (int fy = 0; fy < samples_down; fy++) {
        int cy = fy * image_height / samples_down;
        int ty = cy - patch_radius;
        int by = cy + patch_radius;
        if (ty < 0) ty = 0;
        if (by >= image_height) by = image_height-1;
        for (int fx = 0; fx < samples_across; fx++) {
            int cx = fx * image_width / samples_across;
            int tx = cx - patch_radius;
            int bx = cx + patch_radius;
            if (tx < 0) tx = 0;
            if (bx >= image_width) bx = image_width-1;

            /* scan the patch */
            int index_feature_input = 0;
            for (int y = ty; y < by; y++) {
                for (int x = tx; x < bx;
                     x++, index_feature_input++) {
                    int index_image =
                        ((y*image_width) + x) *
                        image_depth;
                    /* convert from 8 bit to a neuron value */
                    float v =
                        0.25f + (img[index_image]/(2*255.0f));
                    bp_set_input(feature_autocoder,
                                 index_feature_input, v);
                }
            }
            bp_feed_forward_layers(feature_autocoder, 0);
            int index_input_layer =
                (fy * samples_across + fx) *
                no_of_learned_features;
            for (int f = 0; f < no_of_learned_features; f++) {
                bp_set_input(net, index_input_layer+f,
                             bp_get_hidden(feature_autocoder,
                                           0, index_input_layer+f));
            }
        }
    }
    return 0;
}

/**
* @brief Convolve an image with learned features and output
*        the results to an array of floats
* @param samples_across The number of units across in the array of floats (sampling grid resolution)
* @param samples_down The number of units down in the array of floats (sampling grid resolution)
* @param patch_radius The radius of the patch within the float array
* @param image_width Width of the image
* @param image_height Height of the image
* @param image_depth Depth of the image (mono=1, RGB=3)
* @param img Image buffer
* @param layer0_units Number of units in the float array
* @param layer0 float array
* @param feature_autocoder An autocoder containing learned features
* @returns zero on success
*/
int features_convolve_image_to_floats(int samples_across,
                                      int samples_down,
                                      int patch_radius,
                                      int image_width,
                                      int image_height,
                                      int image_depth,
                                      unsigned char * img,
                                      int layer0_units,
                                      float * layer0,
                                      bp * feature_autocoder)
{
    int no_of_learned_features = feature_autocoder->NoOfHiddens;
    
    if (samples_across * samples_down * no_of_learned_features !=
        layer0_units) {
        /* across*down doesn't equal the second layer units */
        return -1;
    }

    if (feature_autocoder->NoOfInputs !=
        patch_radius*patch_radius*4*image_depth) {
        /* the patch size doesn't match the feature
           learner inputs */
        return -2;
    }

    if (feature_autocoder->NoOfInputs !=
        feature_autocoder->NoOfOutputs) {
        /* same number of inputs and outputs */
        return -3;
    }

    for (int fy = 0; fy < samples_down; fy++) {
        int cy = fy * image_height / samples_down;
        int ty = cy - patch_radius;
        int by = cy + patch_radius;
        if (ty < 0) ty = 0;
        if (by >= image_height) by = image_height-1;
        for (int fx = 0; fx < samples_across; fx++) {
            int cx = fx * image_width / samples_across;
            int tx = cx - patch_radius;
            int bx = cx + patch_radius;
            if (tx < 0) tx = 0;
            if (bx >= image_width) bx = image_width-1;

            /* scan the patch */
            int index_feature_input = 0;
            for (int y = ty; y < by; y++) {
                for (int x = tx; x < bx;
                     x++, index_feature_input++) {
                    int index_image =
                        ((y*image_width) + x) *
                        image_depth;
                    /* convert from 8 bit to a neuron value */
                    float v =
                        0.25f + (img[index_image]/(2*255.0f));
                    bp_set_input(feature_autocoder,
                                 index_feature_input, v);
                }
            }
            bp_feed_forward_layers(feature_autocoder, 0);
            int index_layer0 =
                (fy * samples_across + fx) *
                no_of_learned_features;
            for (int f = 0; f < no_of_learned_features; f++) {
                layer0[index_layer0+f] =
                    bp_get_hidden(feature_autocoder,
                                  0, index_layer0+f);
            }
        }
    }
    return 0;
}

/**
* @brief Convolve a first array of floats to a second one
* @param samples_across The number of units across in the second array of floats (sampling grid resolution)
* @param samples_down The number of units down in the second array of floats (sampling grid resolution)
* @param patch_radius The radius of the patch within the first float array
* @param floats_width Width of the image
* @param floats_height Height of the image
* @param floats_depth Depth of the image (mono=1, RGB=3)
* @param layer0 First array of floats
* @param layer1_units Number of units in the second float array
* @param layer1 Second float array
* @param feature_autocoder An autocoder containing learned features
* @returns zero on success
*/
int features_convolve_floats_to_floats(int samples_across,
                                       int samples_down,
                                       int patch_radius,
                                       int floats_width,
                                       int floats_height,
                                       int floats_depth,
                                       float * layer0,
                                       int layer1_units,
                                       float * layer1,
                                       bp * feature_autocoder)
{
    int no_of_learned_features = feature_autocoder->NoOfHiddens;
    
    if (samples_across * samples_down * no_of_learned_features !=
        layer1_units) {
        /* across*down doesn't equal the second layer units */
        return -1;
    }

    if (feature_autocoder->NoOfInputs !=
        patch_radius*patch_radius*4*floats_depth) {
        /* the patch size doesn't match the feature
           learner inputs */
        return -2;
    }

    if (feature_autocoder->NoOfInputs !=
        feature_autocoder->NoOfOutputs) {
        /* same number of inputs and outputs */
        return -3;
    }

    for (int fy = 0; fy < samples_down; fy++) {
        int cy = fy * floats_height / samples_down;
        int ty = cy - patch_radius;
        int by = cy + patch_radius;
        if (ty < 0) ty = 0;
        if (by >= floats_height) by = floats_height-1;
        for (int fx = 0; fx < samples_across; fx++) {
            int cx = fx * floats_width / samples_across;
            int tx = cx - patch_radius;
            int bx = cx + patch_radius;
            if (tx < 0) tx = 0;
            if (bx >= floats_width) bx = floats_width-1;

            /* scan the patch */
            int index_feature_input = 0;
            for (int y = ty; y < by; y++) {
                for (int x = tx; x < bx;
                     x++, index_feature_input++) {
                    int index_image =
                        ((y*floats_width) + x) *
                        floats_depth;
                    bp_set_input(feature_autocoder,
                                 index_feature_input,
                                 layer0[index_image]);
                }
            }
            bp_feed_forward_layers(feature_autocoder, 0);
            int index_layer1 =
                (fy * samples_across + fx) *
                no_of_learned_features;
            for (int f = 0; f < no_of_learned_features; f++) {
                layer1[index_layer1+f] =
                    bp_get_hidden(feature_autocoder,
                                  0, index_layer1+f);
            }
        }
    }
    return 0;
}

/**
* @brief Convolve an array of floats to the input layer of a neural net
* @param samples_across The number of units across in the layer of neurons (sampling grid resolution)
* @param samples_down The number of units down in the layer of neurons (sampling grid resolution)
* @param patch_radius The radius of the patch within the float array
* @param floats_width Width of the image
* @param floats_height Height of the image
* @param floats_depth Depth of the image (mono=1, RGB=3)
* @param layer0 Array of floats
* @param net Neural net to set the inputs for
* @param feature_autocoder An autocoder containing learned features
* @returns zero on success
*/
int features_convolve_floats_to_neurons(int samples_across,
                                        int samples_down,
                                        int patch_radius,
                                        int floats_width,
                                        int floats_height,
                                        int floats_depth,
                                        float * layer0,
                                        bp * net,
                                        bp * feature_autocoder)
{
    int no_of_learned_features = feature_autocoder->NoOfHiddens;
    
    if (samples_across * samples_down * no_of_learned_features !=
        net->NoOfInputs) {
        /* across*down doesn't equal the second layer units */
        return -1;
    }

    if (feature_autocoder->NoOfInputs !=
        patch_radius*patch_radius*4*floats_depth) {
        /* the patch size doesn't match the feature
           learner inputs */
        return -2;
    }

    if (feature_autocoder->NoOfInputs !=
        feature_autocoder->NoOfOutputs) {
        /* same number of inputs and outputs */
        return -3;
    }

    for (int fy = 0; fy < samples_down; fy++) {
        int cy = fy * floats_height / samples_down;
        int ty = cy - patch_radius;
        int by = cy + patch_radius;
        if (ty < 0) ty = 0;
        if (by >= floats_height) by = floats_height-1;
        for (int fx = 0; fx < samples_across; fx++) {
            int cx = fx * floats_width / samples_across;
            int tx = cx - patch_radius;
            int bx = cx + patch_radius;
            if (tx < 0) tx = 0;
            if (bx >= floats_width) bx = floats_width-1;

            /* scan the patch */
            int index_feature_input = 0;
            for (int y = ty; y < by; y++) {
                for (int x = tx; x < bx;
                     x++, index_feature_input++) {
                    int index_image =
                        ((y*floats_width) + x) *
                        floats_depth;
                    bp_set_input(feature_autocoder,
                                 index_feature_input,
                                 layer0[index_image]);                  
                }
            }
            bp_feed_forward_layers(feature_autocoder, 0);
            int index_net_inputs =
                (fy * samples_across + fx) *
                no_of_learned_features;
            for (int f = 0; f < no_of_learned_features; f++) {
                bp_set_input(net, index_net_inputs+f,
                             bp_get_hidden(feature_autocoder,
                                           0, index_net_inputs+f));
            }
        }
    }
    return 0;
}
