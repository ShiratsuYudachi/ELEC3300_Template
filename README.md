# ELEC3300_Template
This is an UNOFFICIAL Project Template for HKUST course ELEC3300. It aims to save time for handling the drivers of peripherals, and allows you to concentrate on project design itself.

This template is not fully tested yet, should you have any difficulties, feel free to post an issue here or approach me through email my itsc cliudp.

## Easy UI


### How to start?

This template provide an easy-to-use UI framework.
To draw the UI, first you need to create a `Screen` as below:

```
Screen mainScreen;
```

An UIElement, like Button, can be easily created and assigned to a screen by creating an object Button as below.

```
Button settingButton(&mainScreen, 170, 50, "SETTING", 40, 40);
```
- &mainScreen mean you assign this button to mainScreen
- 170, 50 represents the x,y coordinates
- "SETTING" is the button text
- 40, 40 is the button weight & height





Next, if you want to do something when this button is pressed, for example switch to another screen called settingScreen, you can assign a callback funtion to the button as below.

```
// this setupUI function is provided, just add several lines inside
void setupUI(){
    ... // setup other UI elements

    settingButton.onPressed = [](){
        settingScreen.setActive();
    };

    ...// setup other UI elements
}


```
- This line of code should be executed ***inside a funtion***. For example, the setupUI function in Core/Src/interface.cpp
- This kind of grammar is called "lambda expression", which is simply defining a function inside another function. To change what will be executed on the button is pressed, simply change the code inside {}
- you can also set the `.onRelased` and `.whilePressing` callback function like this.

```
// In case you don't understand what lambda is doing, the above code is DOING THE SAME THING as below


void onSettingButtonIsPressed(){
    settingScreen.setActive();
}

void setupUI(){
    ... // setup other UI elements

    settingButton.onPressed = onSettingButtonIsPressed;

    ...// setup other UI elements
}


```


(You can check more examples inside Core/Src/interface.cpp.)


### Introduction to available UIElements

#### Button
...
#### Slider
...
#### Touchpad
simply a 2D version of slider
#### Joystick
...


### How EasyUI work?