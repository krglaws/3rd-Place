
function openPage(pageName, button) {
  var i, tabcontent, tablinks;

  tabcontent = document.getElementsByClassName("user-tab-content-container");
  for (i = 0; i < tabcontent.length; i++) {
    tabcontent[i].style.display = "none";
  }

  tablinks = document.getElementsByClassName("user-tab-link");
  for (i = 0; i < tablinks.length; i++) {
    tablinks[i].style.backgroundColor = "#CACBCC";
  }

  document.getElementById(pageName).style.display = "block";
  button.style.backgroundColor = "white";
}

function selectOverview() {
  document.getElementById("default-tab").click();
}

