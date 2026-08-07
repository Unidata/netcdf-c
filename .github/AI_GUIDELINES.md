> This guideline is adapted from the LLVM AI Tool Use Policy found here: https://llvm.org/docs/AIToolPolicy.html, and may be updated as need be. 

# Guideline

NSF Unidata's guideline is that **contributors can use whatever tools they would like to craft their contributions, but there must be a human in the loop.** Contributors *must* read and review all LLM-generated code or text before they ask other project members to review it. The contributor is always the author and is fully accountable for their contributions. Contributors should be sufficiently confident that the contribution is high enough quality that asking for a review is a good use of scarce maintainer time, and they should be able to answer questions about their work during review.

We expect that new contributors will be less confident in their contributions, and our guidance to them is to start with small contributions that they can fully understand to build confidence. We aspire to be a welcoming community that helps new contributors grow their expertise, but learning involves taking small steps, getting feedback, and iterating. Passing maintainer feedback to an LLM doesn’t help anyone grow, and does not sustain our community.

Contributors are expected to be transparent and label contributions that contain substantial amounts of tool-generated content. Our guideline on labelling is intended to facilitate reviews. **Contributors should note tool usage in their pull request description, commit message, or wherever authorship is normally indicated for the work.** 

> For instance, use a commit message trailer like Assisted-by: . This transparency helps the community develop best practices and understand the role of these new tools.

This guideline includes, but is not limited to, the following kinds of contributions:

* Code, usually in the form of a pull request
* Issues or security vulnerabilities
* Comments and feedback on pull requests

# Details

To ensure sufficient self review and understanding of the work, it is strongly recommended that contributors write PR descriptions themselves (if needed, using tools for translation or copy-editing). The description should explain the motivation, implementation approach, expected impact, and any open questions or uncertainties to the same extent as a contribution made without tool assistance.

An important implication of this guideline is that it bans agents that take action in our digital spaces without human approval, such as the GitHub @claude agent. Similarly, automated review tools that publish comments without human review are not allowed. However, an opt-in review tool that keeps a human in the loop is acceptable under this guideline. 

> As another example, using an LLM to generate documentation, which a contributor manually reviews for correctness, edits, and then posts as a PR, is an approved use of tools under this guideline.

# Extractive Contributions

> Prior to the advent of LLMs, open source project maintainers would often review any and all changes sent to the project simply because posting a change for review was a sign of interest from a potential long-term contributor. While new tools enable more development, it shifts effort from the implementor to the reviewer, and our guidelines exist to ensure that we value and do not squander maintainer or contributor time.

The reason for our “human-in-the-loop” contribution guideline is that processing patches, PRs, RFCs, and comments to NSF Unidata's projects is not free – it takes a lot of maintainer time and energy to review those contributions! Sending the *unreviewed* output of an LLM to open source project maintainers extracts work from them in the form of design and code review, so we call this kind of contribution an “extractive contribution”.

Our golden rule is that a contribution should be worth more to the project than the time it takes to review it. These ideas are captured by this quote from the book Working in Public by Nadia Eghbal:

> “When attention is being appropriated, producers need to weigh the costs and benefits of the transaction. To assess whether the appropriation of attention is net-positive, it’s useful to distinguish between extractive and non-extractive contributions. Extractive contributions are those where the marginal cost of reviewing and merging that contribution is greater than the marginal benefit to the project’s producers. In the case of a code contribution, it might be a pull request that’s too complex or unwieldy to review, given the potential upside.” – Nadia Eghbal

Reviewing changes from new contributors is part of growing the next generation of contributors and sustaining the project. We want the suite of NSF Unidata projects to be welcoming and open to aspiring developers who are willing to invest time and effort to learn and grow, because growing our contributor base and recruiting new maintainers helps sustain the project over the long term. Being open to contributions and liberally granting commit access is a big part of how projects at NSF Unidata have grown and successfully been adopted all across the community. We therefore automatically encourage maintainers to spend their time to help new contributors learn.

# Handling Violations

If a maintainer judges that a contribution doesn’t comply with this guideline, they should paste the following response to request changes:

> This PR doesn't appear to comply with our guideline on tool-generated content,
and requires additional justification for why it is valuable enough to the
project for us to review it. Please see our developer guideline on
AI-generated contributions: https://github.com/Unidata/netcdf-c/blob/main/.github/AI_GUIDELINES.md

The best ways to make a change less extractive and more valuable are to reduce its size or complexity or to increase its usefulness to the community. These factors are impossible to weigh objectively, and our project guideline leaves this determination up to the maintainers of the project, i.e. those who are doing the work of sustaining the project.

Maintainers reserve the right to close or defer the review of extractive PRs.

# Copyright

Artificial intelligence systems raise many questions around copyright that have yet to be answered. Our guideline on AI tools is similar to our copyright policy: Contributors are responsible for ensuring that they have the right to contribute code under the terms of our license, typically meaning that either they, their employer, or their collaborators hold the copyright. Using AI tools to regenerate copyrighted material does not remove the copyright, and contributors are responsible for ensuring that such material does not appear in their contributions. Contributions found to violate this guideline will be removed just like any other offending contribution.

# References

Our guideline was informed by and adapted from 

* [LLVM AI Tool Use Policy](https://llvm.org/docs/AIToolPolicy.html) 
* [Slop is the new name for unwanted AI-generated content](https://simonwillison.net/2024/May/8/slop/)
