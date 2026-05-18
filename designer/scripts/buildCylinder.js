
function buildCylinder(){
    const numberOfSlices = 64;
    const radius = 1000/(2*Math.PI);
    const sliceWidth = (2 * Math.PI * radius) / numberOfSlices;
    const overlap = 10;
    const container = document.getElementById('cylinder_container');
    const cylinder = document.createElement('div');
    cylinder.className = "cylinder";
    for(let i = 0; i<numberOfSlices; i++){
        const slice = document.createElement('div');
        slice.className = "slice";
        slice.style.height = "40px";
        slice.style.width = `${sliceWidth+overlap}px`;
        slice.style.left = '200px';
        const angle = (360/numberOfSlices)*i;
        slice.style.transform = `rotateY(${angle}deg) translateZ(${radius}px)`;
        slice.style.maskPosition = `${-i * sliceWidth}px 0px`;
        cylinder.appendChild(slice);
    }
    cylinder.style.transform = `rotateZ(90deg)`;
    container.appendChild(cylinder);
}

buildCylinder();