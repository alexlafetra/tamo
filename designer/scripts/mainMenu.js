
function buildCylinder(){
    const numberOfSlices = 64;
    const radius = 1000/(2*Math.PI);
    const sliceWidth = (2 * Math.PI * radius) / numberOfSlices;
    const overlap = 10;
    const container = document.getElementById('cylinder_container');
    const cylinder = document.createElement('div');
    cylinder.className = "cylinder";
    cylinder.style.left = `${-radius/2-25}px`;
    for(let i = 0; i<numberOfSlices; i++){
        const slice = document.createElement('div');
        slice.className = "slice";
        slice.style.height = "30px";
        slice.style.width = `${sliceWidth+overlap}px`;
        slice.style.left = `187px`;
        // slice.style.left = `${181}px`;
        const angle = (360/numberOfSlices)*i;
        slice.style.transform = `rotateY(${angle}deg) translateZ(${radius}px)`;
        slice.style.maskPosition = `${-i * sliceWidth}px 0px`;
        cylinder.appendChild(slice);
    }
    cylinder.style.transform = `rotateZ(90deg)`;
    container.appendChild(cylinder);
}

function togglePanel(className,trigger){
    const parentContainers = document.getElementsByClassName(className);
    const isOpening = !parentContainers[0].children[0].style.animation.includes('open');
    trigger.style.backgroundColor = isOpening?"blue":null;
    trigger.style.color = isOpening?"white":null;
    for(let container of parentContainers){
        for(let i of container.children){
            i.style.animation = `${isOpening?'open 0.5s':'close 0.1s'} ease forwards`;
        }
    }
    if(className == 'identity_sprite_gif_holder')
        document.getElementById("main_menu").style.gridTemplateRows = `min-content 100px min-content ${isOpening?100:20}px min-content`;
}

buildCylinder();
document.documentElement.style.setProperty('--text-color', 'blue');