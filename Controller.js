function Controller () {

}

Controller.scratching = [];

Controller.ticks = [0,0];
Controller.timer = [];
Controller.init = function () {
    Controller.timer[0] = engine.beginTimer(200, "Controller.resetTicks()");
    Controller.timer[1] = engine.beginTimer(5, "Controller.handleTicks()");
};

Controller.handleTicks = function () {
    for (var i = 0; i < Controller.ticks.length; i++) {
        var ticks = Controller.ticks[i];
        if (ticks) {
            var alpha = 1.0/8;
            var beta = alpha / 32;
            engine.scratchEnable(i+1, 8, 90+2/3, alpha, beta);
            engine.scratchTick(i+1, ticks);
        } else {
            engine.scratchDisable(i+1);
        }
    }
}

Controller.resetTicks = function () {
    Controller.ticks = [0,0];
};

Controller.wheelTurn = function (chan, ctrl, val) {
    Controller.ticks[ctrl - 0x27] += (val - 0x40);
};

