function findSibling(arrow, upOrDown) {

  var voteWrapper = arrow.parentElement.parentElement;

  for (var i = 0; i < voteWrapper.childNodes.length; i++) {
    var child = voteWrapper.childNodes[i];

    if (child['classList'] !== undefined && 
        child.classList.contains('arrow-wrapper')) {

      for (var j = 0; j < child.childNodes.length; j++) {
        var grandChild = child.childNodes[j];

        if (grandChild['classList'] !== undefined &&
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

    if (child['classList'] !== undefined &&
        child.classList.contains('score-wrapper')) {

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

  fetch('/vote', {
    method: 'POST',
    redirect: 'follow',
    credentials: 'same-origin',
    body: "type="+postOrComment+"&direction="+arrowDirection+"&id="+id
  }).then(response => {
    if (response.redirected) {
      window.location.href = response.url;
    }
    else {
      toggleVote(arrow, postOrComment, id, arrowDirection, siblingDirection);
    }
  });
}
