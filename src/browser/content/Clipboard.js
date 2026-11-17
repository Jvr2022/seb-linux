/*
 * Copyright (c) 2026 ETH Zürich, IT Services
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Original code taken and adapted from https://github.com/eqsoft/seb2/blob/master/browser/app/modules/SebBrowser.jsm#L1215.
 */

if (typeof SafeExamBrowser.clipboard === 'undefined') {
	SafeExamBrowser.clipboard = {
		id: Array.from(crypto.getRandomValues(new Uint8Array(16)), function (b) { return b.toString(16).padStart(2, "0"); }).join(""),
		ranges: [],
		text: "",

		clear: function () {
			this.ranges = [];
			this.text = "";
		},

		getContentEncoded: function () {
			var bytes = new TextEncoder().encode(this.text);
			var binary = "";
			for (var offset = 0; offset < bytes.length; offset += 8192) {
				binary += String.fromCharCode.apply(null, bytes.subarray(offset, offset + 8192));
			}
			var base64 = btoa(binary);

			return base64;
		},

		update: function (id, base64) {
			if (this.id != id) {
				var bytes = Uint8Array.from(atob(base64), (m) => m.codePointAt(0));
				var content = new TextDecoder().decode(bytes);

				this.ranges = [];
				this.text = content;
			}
		}
	}
}

if (typeof copySelection === 'undefined') {
	function copySelection(e) {
		if (e.target.contentEditable && e.target.setRangeText) {
			SafeExamBrowser.clipboard.text = e.target.value.substring(e.target.selectionStart, e.target.selectionEnd);
			SafeExamBrowser.clipboard.ranges = [];
		} else {
			var selection = (e.target.ownerDocument || document).defaultView.getSelection();
			var text = "";

			for (var i = 0; i < selection.rangeCount; i++) {
				SafeExamBrowser.clipboard.ranges[i] = selection.getRangeAt(i).cloneContents();
				text += SafeExamBrowser.clipboard.ranges[i].textContent;
			}

			SafeExamBrowser.clipboard.text = text;
		}
	}
}

function editableSelection(target, selection) {
	if (target.disabled || target.readOnly || !selection || !selection.rangeCount) return null;
	var range = selection.getRangeAt(0);
	var node = range.commonAncestorContainer;
	var element = node.nodeType === 1 ? node : node.parentElement;
	var doc = target.ownerDocument || document;
	return doc.designMode === "on" || (element && element.isContentEditable) ? range : null;
}

function cutSelection(e) {
	var target = e.target;
	if (target.disabled || target.readOnly) return false;
	if (typeof target.selectionStart === "number" && typeof target.setRangeText === "function") {
		target.setRangeText("", target.selectionStart, target.selectionEnd, "end");
		return true;
	}
	var doc = target.ownerDocument || document;
	var range = editableSelection(target, doc.defaultView.getSelection());
	if (!range) return false;
	range.deleteContents();
	return true;
}

function pasteContent(e) {
	if (!SafeExamBrowser.clipboard.text && !SafeExamBrowser.clipboard.ranges.length) return false;
	var target = e.target;
	if (target.disabled || target.readOnly) return false;
	if (typeof target.selectionStart === "number" && typeof target.setRangeText === "function") {
		target.setRangeText(SafeExamBrowser.clipboard.text, target.selectionStart, target.selectionEnd, "end");
		return true;
	}
	var doc = target.ownerDocument || document;
	var selection = doc.defaultView.getSelection();
	var range = editableSelection(target, selection);
	if (!range) return false;
	var fragment = doc.createDocumentFragment();
	if (SafeExamBrowser.clipboard.ranges.length) {
		SafeExamBrowser.clipboard.ranges.forEach(function (part) { fragment.appendChild(part.cloneNode(true)); });
	} else {
		fragment.appendChild(doc.createTextNode(SafeExamBrowser.clipboard.text));
	}
	var last = fragment.lastChild;
	range.deleteContents();
	range.insertNode(fragment);
	if (last) range.setStartAfter(last);
	range.collapse(true);
	selection.removeAllRanges();
	selection.addRange(range);
	return true;
}

if (typeof notifyInput === 'undefined') {
	function notifyInput(target, inputType) {
		if (target && typeof target.dispatchEvent === 'function') {
			target.dispatchEvent(new InputEvent("input", { bubbles: true, inputType: inputType }));
		}
	}
}

if (typeof onCopy === 'undefined') {
	function onCopy(e) {
		e.stopImmediatePropagation();
		if (!e.isTrusted || (e.target.tagName === "INPUT" && e.target.type === "password")) {
			e.preventDefault();
			return false;
		}
		try {
			SafeExamBrowser.clipboard.clear();

			copySelection(e);

			CefSharp.PostMessage({ Type: "Clipboard", Id: SafeExamBrowser.clipboard.id, Content: SafeExamBrowser.clipboard.getContentEncoded() });
		} finally {
			e.preventDefault();
		}

		return false;
	}

	window.addEventListener("copy", onCopy, true);
}

if (typeof onCut === 'undefined') {
	function onCut(e) {
		e.stopImmediatePropagation();
		if (!e.isTrusted || (e.target.tagName === "INPUT" && e.target.type === "password")) {
			e.preventDefault();
			return false;
		}
		try {
			SafeExamBrowser.clipboard.clear();

			copySelection(e);
			var changed = cutSelection(e);

			CefSharp.PostMessage({ Type: "Clipboard", Id: SafeExamBrowser.clipboard.id, Content: SafeExamBrowser.clipboard.getContentEncoded() });

			if (changed) notifyInput(e.target, "deleteByCut");
		} finally {
			e.preventDefault();
		}

		return false;
	}

	window.addEventListener("cut", onCut, true);
}


if (typeof onPaste === 'undefined') {
	function onPaste(e) {
		e.stopImmediatePropagation();
		if (!e.isTrusted) {
			e.preventDefault();
			return false;
		}
		try {
			if (pasteContent(e)) notifyInput(e.target, "insertFromPaste");
		} finally {
			e.preventDefault();
		}

		return false;
	}

	window.addEventListener("paste", onPaste, true);
}
