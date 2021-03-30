function openUserTab(pageName, button) {
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


function openCommunityTab(pageName, button) {
  var i, tabcontent, tablinks;

  tabcontent = document.getElementsByClassName("community-tab-content-container");
  for (i = 0; i < tabcontent.length; i++) {
    tabcontent[i].style.display = "none";
  }

  tablinks = document.getElementsByClassName("community-tab-link");
  for (i = 0; i < tablinks.length; i++) {
    tablinks[i].style.backgroundColor = "#CACBCC";
  }

  document.getElementById(pageName).style.display = "block";
  button.style.backgroundColor = "white";
}


function selectOverview() {
  var tab = document.getElementById("default-tab");
  if (tab !== null) {
    tab.click();
  }
}


function logOut() {

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


function deleteUser(user_name) {

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


function findSibling(arrow, upOrDown) {

  var voteWrapper = arrow.parentElement.parentElement;

  for (var i = 0; i < voteWrapper.childNodes.length; i++) {
    var child = voteWrapper.childNodes[i];

    if (child["classList"] !== undefined && 
        child.classList.contains("arrow-wrapper")) {

      for (var j = 0; j < child.childNodes.length; j++) {
        var grandChild = child.childNodes[j];

        if (grandChild["classList"] !== undefined &&
            grandChild.classList.contains(upOrDown)) {

          return grandChild;
        }
      }
    }
  }

  return undefined;
}


function getScoreWrapper(arrow) {

  var voteWrapper = arrow.parentElement.parentElement;

  for (var i = 0; i < voteWrapper.childNodes.length; i++) {
    var child = voteWrapper.childNodes[i];

    if (child["classList"] !== undefined &&
        child.classList.contains("score-wrapper")) {

      return child;
    }
  } 

  return undefined;
}


function decrementScore(arrow) {

  var scoreWrapper = getScoreWrapper(arrow);
  var score = parseInt(scoreWrapper.textContent.trim());

  score--;

  scoreWrapper.textContent = String(score);
}


function incrementScore(arrow) {

  var scoreWrapper = getScoreWrapper(arrow);
  var score = parseInt(scoreWrapper.textContent.trim());

  score++;

  scoreWrapper.textContent = String(score);
}


function toggleVote(arrow, postOrComment, id, arrowDirection, siblingDirection) {

  // make sure sibling arrow is not clicked
  var sibling = findSibling(arrow, siblingDirection);
  if (sibling.classList.contains(siblingDirection + "vote-clicked")) {

    sibling.classList.remove(siblingDirection + "vote-clicked");
    sibling.classList.add(siblingDirection + "vote-notclicked");

    if (arrowDirection === "up") {
      incrementScore(arrow);
    }
    else {
      decrementScore(arrow);
    }
  }

  // is this arrow already clicked?
  var isAlreadyClicked = arrow.classList.contains(arrowDirection + "vote-clicked");

  if (isAlreadyClicked) {
    arrow.classList.remove(arrowDirection + "vote-clicked");
    arrow.classList.add(arrowDirection + "vote-notclicked");

    // change score accordingly
    if (arrowDirection === "up") {
      decrementScore(arrow);
    }
    else {
      incrementScore(arrow);
    }
  }

  else {
    arrow.classList.remove(arrowDirection + "vote-notclicked");
    arrow.classList.add(arrowDirection + "vote-clicked");

    // change score accordingly
    if (arrowDirection === "up") {
      incrementScore(arrow);
    }
    else {
      decrementScore(arrow);
    }
  }
}


function vote(arrow, postOrComment, id) {

  // determine arrow types
  var arrowDirection = arrow.classList.contains("up") ? "up" : "down";
  var siblingDirection = arrowDirection === "up" ? "down" : "up";

  fetch("/vote", {
    method: "POST",
    redirect: "follow",
    credentials: "same-origin",
    body: "type="+postOrComment+"&direction="+arrowDirection+"&id="+id
  }).then(response => {
    if (response.redirected) {
      window.location.href = response.url;
    }
    else if (response.ok) {
      toggleVote(arrow, postOrComment, id, arrowDirection, siblingDirection);
    }
    else {
      console.log(response);
      alert("Error");
    }
  });
}
