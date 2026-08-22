
//followed this tutorial: https://dev.to/whitep4nth3r/the-best-lightdark-mode-theme-toggle-in-javascript-368f

function checkColorTheme(){
    let colorTheme;
    const previousTheme = localStorage.getItem("theme");
    //if there was a previously stored theme, use that
    if(previousTheme){
        colorTheme = previousTheme;
    }
    else{
        const systemSettingDark = window.matchMedia("(prefers-color-scheme: dark)");
        const systemSettingLight = window.matchMedia("(prefers-color-scheme: light)");
        if(systemSettingDark.matches)
            colorTheme = "dark";
        else if(systemSettingLight.matches)
            colorTheme = "light";
        else colorTheme = "light";
    }

    document.getElementsByTagName('html')[0].setAttribute('data-theme',colorTheme);
    const element = document.getElementById("dark_mode_button");
    element.style.marginTop = (colorTheme == "dark")?"15px":"0px";
}

function toggleDarkMode(){
    const element = document.getElementById("dark_mode_button");
    const isLight = (element.style.marginTop == "0px");
    localStorage.setItem("theme", isLight?'dark':'light');
    element.style.marginTop = isLight?"15px":"0px";
    document.getElementsByTagName('html')[0].setAttribute('data-theme',isLight?'dark':'light');
}
