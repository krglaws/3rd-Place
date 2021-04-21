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


function pageInit() {

  processTimestamps();

  var tab = document.getElementById("default-tab");
  if (tab !== null) {
    tab.click();
  }
}


function logOut() {

  fetch("/logout", {
    method: "POST"
  }).then(response => {
    if (response.redirected) {
      window.history.pushState("", "", response.url);
      return response.text();
    }
    else {
      console.log(response);
      alert("Error");
      return null;
    }
  }).then(data => {
    if (data !== null) {
      var tempDom = new DOMParser()
        .parseFromString(data, "text/html");
      document.body.innerHTML = tempDom.body.innerHTML;
    }
  });
}


function subscribe(elem, community_id) {

  fetch("/subscribe", {
    method: "POST",
    body: "id="+community_id
  }).then(response => { 
    if (response.redirected) {
      window.history.pushState("", "", response.url);
      return response.text();
    }
    else if (response.ok) {
      if (elem.textContent == "[subscribe]") {
        elem.textContent = "[unsubscribe]";
      }
      else {
        elem.textContent = "[subscribe]";
      }
      return null;
    }
    else if (response.status == 404) {
      alert("this community does not exist");
    }
    else {
      console.log(response);
      alert("Error");
      return null;
    }
  }).then(data => {
    if (data != null) {
      var tempDom = new DOMParser()
        .parseFromString(data, "text/html");
      document.body.innerHTML = tempDom.body.innerHTML;
    }
  });
}


function deleteObject(objType, id) {

  if (confirm("Are you sure you want to delete this "+objType+"?") == false) {
    return;
  }

  fetch("/delete_"+objType+"?id="+id, {
    method: "POST"
  }).then(response => {
    if (response.redirected) {
      window.history.pushState("", "", response.url);
      return response.text();
    }
    else if (response.status == 403) {
      alert("permission denied");
      return null;
    }
    else if (response.status == 404) {
      alert(objType+" does not exist");
      return null;
    }
    else {
      console.log(response);
      alert("Error");
      return null;
    }
  }).then(data => {
    if (data != null) {
      var tempDom = new DOMParser()
      .parseFromString(data, "text/html");
      document.body.innerHTML = tempDom.body.innerHTML;
    }
  });
}


function deleteModerator(user_id, community_id) {

  if (confirm("Are you sure you want to remove this moderator?") == false) {
    return;
  }

  fetch("/delete_moderator?uid="+user_id+"&cid="+community_id, {
    method: "POST"
  }).then(response => {
    if (response.redirected) {
      window.history.pushState("", "", response.url);
      return response.text();
    }
    else if (response.status == 403) {
      alert("permission denied");
      return null;
    }
    else if (response.status == 404) {
      alert(objType+" does not exist");
      return null;
    }
    else {
      console.log(response);
      alert("Error");
      return null;
    }
  }).then(data => {
    if (data != null) {
      var tempDom = new DOMParser()
      .parseFromString(data, "text/html");
      document.body.innerHTML = tempDom.body.innerHTML;
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


function processTimestamps() {

  var timeStampList = document.getElementsByClassName("utc-timestamp");

  for (var i = 0; i < timeStampList.length; i++) {

    var utcElem = timeStampList[i];
    var date = new Date();

    var postTime = utcElem.innerText;
    var currTime = Math.floor(date.getTime() / 1000);

    var delta = currTime - postTime;
    console.log(delta);
    var timeStr = "";

    if (delta < 60) {
      utcElem.innerText = delta + " seconds ago";
    }
    else if (delta < 3600) {
      utcElem.innerText = (delta / 60) + " minutes ago";
    }
    else if (delta < 86400) {
      utcElem.innerText = (delta / 3600) + " hours ago";
    }
    else {
      date.setUTCSeconds(utcElem.innerText);
      utcElem.innerText = date.toDateString();
    }
  }
}
