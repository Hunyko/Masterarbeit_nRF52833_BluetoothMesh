# nrfConnect_MA

## Model Implementation Overview
- [x] Test
- [] Hit
    - [x] Basic Framework
    - [] Information encoding&decoding
- [] Heartbeat
    - [x] Basic Framework
    - [] Information encoding&decoding
- [] Playerstatus
    - [] Basic Framework
    - [] Information encoding&decoding
- [] Device Modification
    - [] Basic Framework
    - [] Information encoding&decoding
- [] Game Modidifcation
    - [] Basic Framework
    - [] Information encoding&decoding
- [] Player ID
- [] Partner Address

Currently all models have to be provisioned via the mesh app.
Auto provisioning via the nodes own software during startup causes a hardfault.
Provisioning via a separate board is not feasible as there are lots of parameteters which need to be initialized during provisioning.
These parameters depend on the board provisioned which at the moment is not able to communicate the setup it needs.

Check if the game and device configuration can be merged into one model to reduce complexity.


### Test Model
This model is used for testing, debugging and similar purposes. It is based on the SIG OnOff model.
Status is marked as complete even though this model will always be updated as required for testing.

### Hit Model
- Basic Framework is implemented.
- Hit message can be sent. PlayerID used is set in the software.

### Heartbeat Model
Not to be mixed up with the SIG defined Heartbeat function.
- Basic Framework is implemented.
- Heartbeat does not automatically start following provisioning. This needs to be investigated.

### Playerstatus Model
- TBD

### Device Modification Model
- TBD

### Game Modification Model
- TBD

### Player ID Model
- TBD -  Not yet sure if this can be implemented into another model to reduce complexity.

### Partner Address  Model
- TBD -  Not yet sure if this can be implemented into another model to reduce complexity.


Next Steps:
- Create Model Handler Copies to test all vendor models one by one.
    Hit Model Handler is next.