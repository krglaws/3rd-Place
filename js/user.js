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

function logOut () {

  fetch("/logout", {
    method: "POST",
    redirect: "follow",
    credentials: "same-origin"
  }).then(response => {
    if (response.redirected) {
      window.location.href = response.url;
    }
    else {
      console.log(response);
      alert("Error");
    }
  });
}

function deleteAccount (user_name) {

  if (confirm("Are you sure you want to delete your account?") == false) {
    return;
  }

  fetch("/delete_user?user_name="+user_name, {
    method: "DELETE",
    redirect: "follow",
    credentials: "same-origin",
  }).then(response => {
    if (response.redirected) {
      window.location.href = response.url;
    }
    else if (response.status == 403) {
      alert("Permission denied");
    }
    else if (response.status == 404) {
      alert("User does not exist");
    }
    else {
      console.log(response);
      alert("Error");
    }
  });
}
