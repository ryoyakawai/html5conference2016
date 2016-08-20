window.addEventListener("resize", function(event){
resizewindow();
});
function resizewindow() {
    var synthw=1024, synthh=800;
    var scalew = window.innerWidth/synthw,
        scaleh = window.innerHeight/synthh;
    //var s=parseInt(100*(scalew<scaleh ? scalew : scaleh))/100;
    var s=parseInt(100*scalew, 10)/100;
    document.getElementById("synth").style.setProperty("transform", "scale("+s+","+s+")");
}
resizewindow();

function fireEvent(elem) {
    var target = document.querySelector("#"+elem);
    var event = document.createEvent("MouseEvents");
    event.initEvent("mousedown", false, true);
    target.dispatchEvent(event);
}