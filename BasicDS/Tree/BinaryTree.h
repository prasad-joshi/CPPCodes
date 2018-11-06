#pragma once

#include <queue>
#include <cmath>

template <typename T>
class BinaryTree {
public:
	struct Node {
		Node(Node* parent, T data) : parentp(parent), data_(std::move(data)) {
		
		}

		~Node() {
			Free();
		}

		void InsertLeft(Node* nodep) {
			leftp = nodep;
		}

		void InsertRight(Node* nodep) {
			rightp = nodep;
		}

		friend std::ostream& operator << (std::ostream& os, const Node& node) {
			os << node.data_;
			return os;
		}

		Node* parentp{nullptr};
		Node* leftp{nullptr};
		Node* rightp{nullptr};
		T data_{};

	private:
		void Free() {
			if (leftp) {
				delete leftp;
				leftp = nullptr;
			}
			if (rightp) {
				delete rightp;
				rightp = nullptr;
			}
		}
	};

	class PreorderIterator {
	public:
		PreorderIterator(Node* nodep) : nodep_(nodep) {
		}

		Node& operator *() {
			return *nodep_;
		}

		Node* operator ->() {
			return nodep_;
		}

		Node* operator &() {
			return nodep_;
		}

		bool operator == (const PreorderIterator& rhs) {
			return nodep_ == rhs.nodep_;
		}

		bool operator != (const PreorderIterator& rhs) {
			return nodep_ != rhs.nodep_;
		}

		PreorderIterator& operator++() {
			if (not nodep_) {
				return *this;
			}

			if (nodep_->leftp) {
				nodep_ = nodep_->leftp;
				return *this;
			} else if (nodep_->rightp) {
				nodep_ = nodep_->rightp;
				return *this;
			}

			if (nodep_->parentp->leftp == nodep_) {
				while (nodep_->parentp and nodep_->parentp->leftp == nodep_ and not 
					nodep_ = nodep_->parentp;
				}
				if (nodep_->parentp and nodep_->parentp->rightp) {
					nodep_ = nodep_->parentp->rightp;
				} else {
					nodep_ = nullptr;
				}
				return *this;
			}

			return *this;
		}

	private:
		Node* nodep_;
	};

	class InorderIterator {
	public:
		InorderIterator(Node* rootp) {
			if (not rootp) {
				return;
			}
			nodep_ = LeftMostChild(rootp);
		}

		Node& operator *() {
			return *nodep_;
		}

		Node* operator ->() {
			return nodep_;
		}

		Node* operator &() {
			return nodep_;
		}

		bool operator == (const InorderIterator& rhs) {
			return nodep_ == rhs.nodep_;
		}

		bool operator != (const InorderIterator& rhs) {
			return nodep_ != rhs.nodep_;
		}

		InorderIterator& operator++() {
			if (not nodep_) {
				return *this;
			}

			if (nodep_->rightp) {
				nodep_ = LeftMostChild(nodep_->rightp);
				return *this;
			}
			
			if (nodep_->parentp) {
				while (nodep_->parentp and nodep_->parentp->rightp == nodep_) {
					nodep_ = nodep_->parentp;
				}

				if (nodep_->parentp) {
					nodep_ = nodep_->parentp;
					return *this;
				}
			}

			nodep_ = nullptr;
			return *this;
		}
	private:
		Node* LeftMostChild(Node* nodep) {
			if (not nodep) {
				return nullptr;
			}
			while (nodep->leftp) {
				nodep = nodep->leftp;
			}
			return nodep;
		}
	private:
		Node* nodep_{nullptr};
	};

	class BfsIterator {
	public:
		BfsIterator(Node* nodep) : nodep_(nodep) {
			AddChildrenToVisit(nodep_);
		}

		BfsIterator& operator++() {
			if (to_visit_.empty()) {
				nodep_ = nullptr;
				return *this;
			}

			nodep_ = to_visit_.front();
			to_visit_.pop();
			AddChildrenToVisit(nodep_);
			return *this;
		}

		Node& operator * () {
			return *nodep_;
		}

		Node* operator -> () {
			return nodep_;
		}

		Node* operator & () {
			return nodep_;
		}

		bool operator == (const BfsIterator& rhs) {
			return nodep_ == rhs.nodep_;
		}

		bool operator != (const BfsIterator& rhs) {
			return nodep_ != rhs.nodep_;
		}

	private:
		void AddChildrenToVisit(Node* nodep) {
			if (not nodep) {
				return;
			}
			if (nodep->leftp) {
				to_visit_.emplace(nodep->leftp);
			}
			if (nodep->rightp) {
				to_visit_.emplace(nodep->rightp);
			}
		}

	private:
		Node* nodep_{nullptr};
		std::queue<Node*> to_visit_;
	};
public:
	virtual ~BinaryTree() {
		if (rootp) {
			delete rootp;
		}
	}

	virtual bool IsEmpty() {
		return not rootp->leftp and not rootp->rightp;
	}

	virtual InorderIterator begin() {
		/* TODO: InorderIterator exposes node pointer - avoid it */
		return InorderIterator(rootp);
	}

	virtual InorderIterator end() {
		return InorderIterator(nullptr);
	}

	virtual Node* NewNode(Node* parentp, T data) {
		auto nodep = new Node(parentp, std::move(data));
		if (not nodep) {
			throw std::bad_alloc();
		}
		return nodep;
	}

	virtual void Insert(T data) {
		if (not rootp) {
			auto new_nodep = NewNode(nullptr, std::move(data));
			rootp = new_nodep;
			return;
		}

		/* Using BfsIterator for BinaryTree creates balanced tree */
		auto eit = BfsIterator(nullptr);
		for (auto it = BfsIterator(rootp); it != eit; ++it) {
			auto parentp = &it;
			if (not parentp->leftp) {
				auto new_nodep = NewNode(parentp, std::move(data));
				parentp->leftp = new_nodep;
				break;
			}
			
			if (not parentp->rightp) {
				auto new_nodep = NewNode(parentp, std::move(data));
				parentp->rightp = new_nodep;
				break;
			}
		}
	}

	virtual ssize_t Height() const noexcept {
		return Height(rootp);
	}

	virtual bool IsBalanced() const noexcept {
		bool balanced = true;
		IsBalanced(rootp, balanced);
		return balanced;
	}

	friend std::ostream& operator << (std::ostream& os, BinaryTree<T>& tree) {
		for (const auto& node : tree) {
			os << node << " ";
		}
		return os;
	}

protected:
	virtual ssize_t Height(Node* nodep) const noexcept {
		if (not nodep) {
			return 0;
		}
		auto lh = Height(nodep->leftp);
		auto rh = Height(nodep->rightp);
		return std::max(lh, rh) + 1;
	}

	virtual ssize_t IsBalanced(Node* nodep, bool& balanced) const noexcept {
		if (not nodep) {
			return 0;
		}
		auto lh = IsBalanced(nodep->leftp, balanced);
		auto rh = IsBalanced(nodep->rightp, balanced);
		if (std::abs(lh - rh) > 1) {
			balanced = false;
		}
		return std::max(lh, rh) + 1;
	}

protected:
	Node* rootp{nullptr};
};

template <typename T>
class BST : public virtual BinaryTree<T> {
public:
	virtual ~BST() {
	
	}

	virtual void Insert(T data) override {
		if (not this->rootp) {
			auto new_nodep = this->NewNode(nullptr, std::move(data));
			this->rootp = new_nodep;
			return;
		}

		auto nodep = this->rootp;
		while (nodep != nullptr) {
			if (nodep->data_ < data) {
				if (nodep->rightp) {
					nodep = nodep->rightp;
					continue;
				}
				nodep->rightp = this->NewNode(nodep, std::move(data));
				break;
			} else {
				if (nodep->leftp) {
					nodep = nodep->leftp;
					continue;
				}
				nodep->leftp = this->NewNode(nodep, std::move(data));
				break;
			}
		}
	}

private:
};
