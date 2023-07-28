#include "Data.h"
#include "Rendering.h"
#include "Debug.h"
#include "Serialization.h"
#include "EngineUI.h"


namespace Data {

    //Globals
    Entities entities;
    RenderedModels renderedModels;

    void newEntity(std::string typePath) {

        std::random_device rd; // obtain a random seed from hardware
        std::mt19937 gen(rd()); // seed the generator
        std::uniform_int_distribution<> dis(1, 500); // define the range of values
        int numEntities = Data::numEntities;
        int newEntityId = -1;


        for (int i = 0; i < numEntities; i++) {

            if (entities.id[i] != 0) { continue; }

            //Found a slot for the entity to be stored in, give it an id
            //Generate a random number for the entity's ID; if number has been already been used, start again

            int randomNumber = dis(gen);
            bool isUniqueId = true;
            for (int j = 0; j < numEntities; j++) { //Check this id hasn't been used before
                if (entities.id[j] == randomNumber) {
                    int thisnum = entities.id[j];
                    isUniqueId = false;
                    newEntity(typePath);
                    break;
                }
            }

            if (isUniqueId) {
                //Id is unique, assign it
                entities.id[i] = randomNumber;
                entities.typePath[i] = typePath;
                entities.renderedModelId[i] = Rendering::loadModelDynamically(Serialization::yamlFileAsStringMap(typePath)["model_path"]);
                glm::mat4 transform = glm::mat4(1.0f); // Initialize the matrix to the identity matrix
                transform = glm::translate(transform, glm::vec3(0.0f, 0.0f, 0.0f)); // Set the position to (0, 0, 0)
                transform = glm::scale(transform, glm::vec3(1.0f, 1.0f, 1.0f)); // Set the scale to (1, 1, 1)
                entities.transform[i] = transform;
                newEntityId = randomNumber;
            }

            break;

        }

       

        //Automatically select the newly added item in the UI
      //  EngineUI::currentSelectedItem = std::to_string(newEntityId) + ".entity";
        //debug.log(EngineUI::currentSelectedItem + " added");
    }

    //Find the next empty entry in an array of ids and fills it with a unique id, returning the key of the entry
    int initialiseNewEntry(int id[], int size) {

        std::random_device rd; // obtain a random seed from hardware
        std::mt19937 gen(rd()); // seed the generator
        std::uniform_int_distribution<> dis(1, 500); // define the range of values

        for (int i = 0; i < size; i++) {

            //Found a slot for the entity to be stored in
            if (id[i] != 0) { continue; }
            
            //Generate a random number for the entry's ID; if number has been already been used, start again
            int randomNumber = dis(gen);
            bool isUniqueId = true;
            for (int j = 0; j < size; j++) { //Check this id hasn't been used before
                if (id[j] == randomNumber) {
                    int thisnum = id[j];
                    isUniqueId = false;
                    initialiseNewEntry(id, size);
                    break;
                }
            }

            if (isUniqueId) { 
                id[i] = randomNumber;
                return i; 
            }

            break;

        }


    }

    int getEntityIndexFromId(int entityId) {
        int entityIndex = 0;
        for (int i = 0; i < Data::numEntities; ++i) {
            if (Data::entities.id[i] == entityId) {
                entityIndex = i;
                break;
            }
        }
        return entityIndex;
    }


}